#include "Physics.h"
#include "Utils/Constants.h"

#include <ranges>
#include <tuple>

void Physics::addRigidBody(std::weak_ptr<component::RigidBody> rigid_body)
{
    rigid_bodys_.push_back(rigid_body);
}

void Physics::update(float delta_time)
{
    constexpr const float GRAVITY = 9.81f;

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

        if (collider->collideWith(*water_collider))
        {
            rigid_body->addForce([](const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, const glm::vec3 &,
                                    float mass) { return std::make_tuple(1.05f * mass * GRAVITY * UP, glm::vec3{}); });
        }

        rigid_body->addForce([](const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, const glm::vec3 &,
                                float mass) { return std::make_tuple(-mass * GRAVITY * UP, glm::vec3{}); });
        rigid_body->updatePhysics(delta_time);

        const auto world_transform = transform->resolve();

        const auto world_position = glm::vec3(world_transform[3]);
        transform->translate(rigid_body->position_ - world_position);

        // TODO rotation
    }
}
