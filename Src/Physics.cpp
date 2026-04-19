#include "Physics.h"

#include <ranges>
#include <tuple>
#include <unordered_set>

#include "GameObject.h"
#include "Utils/Constants.h"
#include "Utils/Math.h"

void Physics::addRigidBody(std::weak_ptr<component::RigidBody> rigid_body)
{
    rigid_bodys_.push_back(rigid_body);
}

void Physics::addCollider(std::weak_ptr<component::Collider> collider, bool is_water)
{
    if (is_water)
        water_collider_ = collider;
    else
        colliders_.push_back(collider);
}

void Physics::update(float delta_time)
{
    std::erase_if(rigid_bodys_, [](const auto &wp) { return wp.expired(); });
    std::erase_if(colliders_, [](const auto &wp) { return wp.expired(); });
    
    std::unordered_set<std::shared_ptr<component::Collider>> had_water_collision;
    
    const auto water_collider = water_collider_.lock();

    for (auto [i, rigid_body_ptr] : rigid_bodys_ | std::views::enumerate)
    {
        auto rigid_body = rigid_body_ptr.lock();
        auto collider = rigid_body->collider_.lock();
        auto transform = collider->transform_.lock();

        if (rigid_body->is_static_)
        {
            continue;
        }

        if (collider->collideWith(*water_collider))
        {
            had_water_collision.insert(collider);

            // Archimedes' force simulation
            rigid_body->addForce([](const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, const glm::quat &,
                                    float mass) { return std::make_tuple(mass * GRAVITY * UP, glm::vec3{}); });
        }

        // Gravity
        rigid_body->addForce([](const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, const glm::quat &,
                                float mass) { return std::make_tuple(-mass * GRAVITY * UP, glm::vec3{}); });

        // Javelin stabilizer
        rigid_body->addForce([](const glm::vec3 &velocity, const glm::vec3 &, const glm::vec3 &angular_velocity,
                                const glm::quat &orientation, float) {
            const glm::vec3 forward = getForwardVector(orientation);

            const float tail_offset = 1.5f; // tune: distance from CM to tail
            const glm::vec3 application_point = -forward * tail_offset;

            const glm::vec3 v_tail = velocity + glm::cross(angular_velocity, application_point);

            const glm::vec3 v_perp = v_tail - glm::dot(v_tail, forward) * forward;

            const float drag_coeff = 5.0f; // tune: higher = faster alignment
            const glm::vec3 force = -drag_coeff * v_perp;

            return std::make_tuple(force, application_point);
        });

        rigid_body->updatePhysics(delta_time);

        const auto world_transform = transform->resolve();

        const auto world_position = glm::vec3(world_transform[3]);
        transform->translate(rigid_body->position_ - world_position);

        transform->setRotation(rigid_body->orientation_);
        // TODO collision detection + solving
    }

    const auto water_id = water_collider->getOwner()->getId();
    std::vector<std::shared_ptr<GameObject>> to_detach;
    for (size_t i = 0; i < colliders_.size(); ++i) {
        const auto &collider = colliders_[i].lock();
        
        if (collider->isDisabled())
            continue;

        if (had_water_collision.contains(collider)) {
            if (collider->callCollisionCallbacks(water_id))
                to_detach.push_back(collider->getOwner());
        }

        for (size_t j = i + 1; j < colliders_.size(); ++j) {
            const auto &other_collider = colliders_[j].lock();

            if (other_collider->isDisabled())
                continue;

            if (collider->collideWith(*other_collider))
            {
                if (collider->callCollisionCallbacks(other_collider->getOwner()->getId()))
                    to_detach.push_back(collider->getOwner());
                if (other_collider->callCollisionCallbacks(collider->getOwner()->getId()))
                    to_detach.push_back(other_collider->getOwner());
            }
        }
    }

    for (const auto &game_object : to_detach)
    {
        game_object->detach();
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
