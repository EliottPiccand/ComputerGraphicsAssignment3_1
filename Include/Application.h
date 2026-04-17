#pragma once

#include <memory>
#include <optional>

#include "Clock.h"
#include "Components/Camera3D.h"
#include "Components/FreeViewControls.h"
#include "GameObject.h"
#include "Singleton.h"
#include "Window.h"

class Application
{
  public:
    Application();
    void run();

  private:
    Clock clock_;

    std::unique_ptr<Window> window_;

    std::shared_ptr<GameObject> scene_root_;

    std::weak_ptr<component::Camera3D> free_view_camera_;
    std::weak_ptr<component::FreeViewControls> free_view_controls_;

    bool free_view_override_;
    View main_view_;
    std::weak_ptr<component::Camera3D> top_view_camera_;
    std::weak_ptr<component::Camera3D> cannon_camera_;
    std::optional<std::weak_ptr<component::Camera3D>> last_cannonball_camera_;

    void initializeOpenGL();

    void update(float delta_time);
    void render() const;

    void restart();
};
