#pragma once

#include <memory>
#include <vector>

#include "Components/Collider.h"
#include "Components/RigidBody.h"

class Physics
{
  public:
    static inline std::weak_ptr<component::Collider> water_collider_;

    static void addRigidBody(std::weak_ptr<component::RigidBody> rigid_body);
    static void update(float delta_time);

  private:
    static inline std::vector<std::weak_ptr<component::RigidBody>> rigid_bodys_;
};
