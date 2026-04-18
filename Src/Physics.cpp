#include "Physics.h"

#include <ranges>
#include <tuple>

#include "GameObject.h"
#include "Utils/Constants.h"

void Physics::addRigidBody(std::weak_ptr<component::RigidBody> rigid_body)
{
    rigid_bodys_.push_back(rigid_body);
}

void Physics::update(float delta_time)
{
    const auto water_collider = water_collider_.lock();
    std::erase_if(rigid_bodys_, [](const auto &wp) { return wp.expired(); });

    for (auto [i, rigid_body_ptr] : rigid_bodys_ | std::views::enumerate)
    {
        auto rigid_body = rigid_body_ptr.lock();
        auto collider = rigid_body->collider_.lock();
        auto transform = collider->transform_.lock();

        if (rigid_body->is_static_)
        {
            continue;
        }

        bool collided_with_water = false;
        if (collider->collideWith(*water_collider))
        {
            collided_with_water = true;

            rigid_body->addForce([](const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, const glm::vec3 &,
                                    float mass) { return std::make_tuple(1.05f * mass * GRAVITY * UP, glm::vec3{}); });
        }

        rigid_body->addForce([](const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, const glm::vec3 &,
                                float mass) { return std::make_tuple(-mass * GRAVITY * UP, glm::vec3{}); });
        rigid_body->updatePhysics(delta_time);

        const auto world_transform = transform->resolve();

        const auto world_position = glm::vec3(world_transform[3]);
        transform->translate(rigid_body->position_ - world_position);

        transform->setRotation(glm::quat(rigid_body->angular_position_));
        // TODO collision detection + solving

        if (collided_with_water)
        {
            const auto water_id = water_collider->getOwner()->getId();

            for (const auto &callback : rigid_body->collision_callbacks_)
            {
                callback(water_id);
            }
        }
    }
}

std::vector<glm::vec3> Physics::simulateCannonballTrajectory(const glm::vec3 &initial_position,
                                                             const glm::vec3 &initial_velocity)
{
    constexpr const float DT = 0.05f;

    std::vector<glm::vec3> positions;
    positions.push_back(initial_position);

    glm::vec3 position = initial_position;
    glm::vec3 velocity = initial_velocity;

    while (glm::dot(position, UP) > 0.0f)
    {
        const auto acceleration = -GRAVITY * UP;
        velocity += acceleration * DT;
        position += velocity * DT;
        positions.push_back(position);
    }

    return positions;
}
