#pragma once

#include <memory>
#include <variant>

#include <Lib/glm.h>

#include "Components/Component.h"
#include "Components/Transform.h"

class Physics;

namespace component
{

class RigidBody;

class Collider : public Component
{
  public:
    struct AABB
    {
        glm::vec3 half_size;
        glm::vec3 center;
    };

    using Type = std::variant<AABB>;

    Collider(Type type);

    bool collideWith(const Collider &other) const;
    bool collideWithAABB(const Collider &other) const;

    void initialize() override;
    void update(float delta_time) override;
    bool render() const override;

  private:
    friend Physics;
    friend RigidBody;

    std::weak_ptr<Transform> transform_;

    Type type_;
    AABB aabb_;
};

} // namespace component
