#pragma once

#include <memory>

#include <Lib/glm.h>

#include "Components/Camera3D.h"
#include "Components/Component.h"
#include "Components/Transform.h"
#include "GameObject.h"

namespace component
{

class CannonPlayerController : public Component
{
  public:
    CannonPlayerController(std::weak_ptr<Transform> cannon_barrel_transform, std::weak_ptr<Transform> target_transform,
                           std::weak_ptr<Camera3D> camera);

    void initialize() override;
    void update(float delta_time) override;
    bool render() const override;

  private:
    glm::vec3 getShootingInitialVelocity(const glm::vec3 &target) const;

    std::weak_ptr<Transform> transform_;
    std::weak_ptr<Transform> barrel_transform_;
    std::weak_ptr<Transform> target_transform_;

    std::weak_ptr<Camera3D> camera_;

    bool aiming_;
    glm::vec3 cannon_ball_initial_velocity_;
    GameObjectId shooter_id_;
};

} // namespace component
