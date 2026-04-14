#include "Physics.h"

#include <ranges>

void Physics::addRigidBody(std::weak_ptr<component::RigidBody> rigid_body)
{
    rigid_bodys_.push_back(rigid_body);
}

void Physics::update(float delta_time)
{
    (void)delta_time;

    std::erase_if(rigid_bodys_, [](const auto &wp) { return wp.expired(); });

    for (auto [i, rigid_body_ptr] : rigid_bodys_ | std::views::enumerate)
    {
        auto rigid_body = rigid_body_ptr.lock();
        auto collider = rigid_body->collider_.lock();
        auto transform = collider->transform_.lock();
    
        const auto world_transform = transform->resolve();

        const auto world_position = glm::vec3(world_transform[3]);
        transform->translate(rigid_body->position_ - world_position);
    
        // TODO rotation
    }
}
