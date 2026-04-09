#pragma once

#include <cstdint>
#include <memory>

#include "Clock.h"
#include "Components/Camera3D.h"
#include "GameObject.h"
#include "Window.h"

class Application
{
  private:
    Clock clock;

    std::unique_ptr<Window> window;

    std::shared_ptr<GameObject> sceneRoot;
    std::weak_ptr<component::Camera3D> activeCamera;

    void update(float deltaTime);
    void render() const;

    void restart();
    void onResize(uint32_t width, uint32_t height);

  public:
    Application();
    void run();
};
