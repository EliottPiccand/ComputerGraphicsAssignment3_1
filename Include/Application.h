#pragma once

#include <memory>

#include "Clock.h"
#include "Components/FreeViewControls.h"
#include "GameObject.h"
#include "Window.h"

class Application
{
  private:
    Clock clock;

    std::unique_ptr<Window> window;

    std::shared_ptr<GameObject> sceneRoot;

    std::weak_ptr<component::FreeViewControls> freeViewControls;

    void initializeOpenGL();

    void update(float deltaTime);
    void render() const;

    void restart();

  public:
    Application();
    void run();
};
