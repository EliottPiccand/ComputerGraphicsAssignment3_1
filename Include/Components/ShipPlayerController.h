#pragma once

#include <memory>

#include "Components/Component.h"
#include "Components/RigidBody.h"
#include "Components/Transform.h"
#include "Utils/SpeedState.h"

namespace component
{

class ShipPlayerController : public Component
{
  public:
    ShipPlayerController();

    void initialize() override;
    void update(float delta_time) override;

  private:
    std::weak_ptr<Transform> transform_;
    std::weak_ptr<RigidBody> rigid_body_;

    SpeedState speed_state_;
};

} // namespace component
