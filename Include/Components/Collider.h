#pragma once

#include <memory>
#include <variant>
#include <vector>

#include <Lib/glm.h>

#include "Components/Component.h"
#include "Components/Transform.h"
#include "GameObject.h"

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

    struct ConvexPolyhedron
    {
        std::vector<glm::vec3> vertices;
        std::vector<glm::uvec3> faces;
    };

    using Type = std::variant<AABB, ConvexPolyhedron>;

    Collider(Type type, bool is_water = false);

    bool collideWith(const Collider &other) const;
    bool collideWithAABB(const Collider &other) const;

    /// return whether the object should be detached
    using CollisionCallback = std::function<bool(GameObjectId)>;
    void addCollisionCallback(CollisionCallback callback);
    /// return whether the object should be detached
    bool callCollisionCallbacks(GameObjectId game_object_id) const;

    void initialize() override;
    void update(float delta_time) override;
    bool render() const override;

  private:
    friend Physics;
    friend RigidBody;

    std::weak_ptr<Transform> transform_;

    Type type_;
    AABB aabb_;

    bool is_water_;
    std::vector<CollisionCallback> collision_callbacks_;
};

} // namespace component
