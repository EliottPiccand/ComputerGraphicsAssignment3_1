#include "Components/RigidBody.h"

#include "GameObject.h" // IWYU pragma: keep

using namespace component;

RigidBody::RigidBody() : is_static_(true)
{
}

RigidBody::RigidBody(float mass, glm::mat3 inertia)
    : is_static_(false), mass_(mass), inverse_mass_(1.0f / mass), inverse_inertia_(glm::inverse(inertia)),
      velocity_({}), angular_velocity_({})
{
}

void RigidBody::addForce(Force force)
{
    forces_.push_back(force);
}

void RigidBody::initialize()
{
    GET_COMPONENT(Collider, collider_, RigidBody);

    const auto &transform = collider_.lock()->transform_.lock()->resolve();
    position_ = glm::vec3(transform[3]);
}

void RigidBody::updatePhysics(float delta_time)
{
    glm::vec3 forces_sum{};
    glm::vec3 torques_sum{};
    for (const auto &force_callback : forces_)
    {
        const auto [force, application_point] =
            force_callback(velocity_, position_, angular_velocity_, angular_position_, mass_);
        forces_sum += force;
        torques_sum += glm::cross(application_point, force);
    }
    forces_.clear();

    const glm::vec3 acceleration = inverse_mass_ * forces_sum;
    velocity_ += acceleration * delta_time;
    position_ += velocity_ * delta_time;

    const glm::vec3 angular_acceleration = inverse_inertia_ * torques_sum;
    angular_velocity_ += angular_acceleration * delta_time;
    angular_position_ += angular_velocity_ * delta_time;
}
