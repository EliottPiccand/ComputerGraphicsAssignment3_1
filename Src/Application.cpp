#include "Application.h"

#include <memory>

#include <GLFW/glfw3.h>

#include "Assets/AssetLoader.h"
#include "Assets/Mesh.h"
#include "Components/Camera3D.h"
#include "Components/MeshInstance.h"
#include "Components/Transform.h"
#include "Events/EventQueue.h"
#include "Events/WindowResized.h"
#include "Input.h"
#include "Utils/Profiling.h"
#include "Utils/Random.h"
#include "Window.h"

Application::Application()
{
    ProfileScope;

    Random::initialize();
    window = std::make_unique<Window>();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    Input::initialize(*window);
    Input::bindKey(Input::Action::ToggleFullScreen, GLFW_KEY_F11);
    Input::bindMouseButton(Input::Action::UIClick, GLFW_MOUSE_BUTTON_1);

    AssetLoader::load<Mesh>("Models/vicking_room.obj");

    sceneRoot = std::make_shared<GameObject>();

    auto vikingRoom = sceneRoot->addChild();
    vikingRoom->addComponent<component::Transform>();
    vikingRoom->addComponent<component::MeshInstance>(AssetLoader::get<Mesh>("Models/vicking_room.obj"));

    auto perspectiveCamera = sceneRoot->addChild();
    perspectiveCamera->addComponent<component::Transform>(glm::vec3{2.0f, 2.0f, 2.0f});
    activeCamera = perspectiveCamera->addComponent<component::Camera3D>(component::Camera3D::Perspective{
        .fov = 45.0f,
        .near = 0.1f,
        .far = 100.0f,
        .lookAt = {0.0f, 0.0f, 0.0f},
    });

    restart();

    sceneRoot->initialize();
}

void Application::run()
{
    while (!window->shouldClose())
    {
        const float deltaTime = clock.tick();
        if (deltaTime > 1.0f)
        {
            continue;
        }

        update(deltaTime);
        render();

        window->endFrame();

        ProfilingEndFrame;
    }
}

void Application::update(float deltaTime)
{
    ProfileScope;

    EventQueue::processAll();

    Input::update();

    if (Input::getState(Input::Action::ToggleFullScreen) == Input::State::JustReleased)
    {
        window->toggleFullscreen();
    }

    sceneRoot->update(deltaTime);
}

void Application::render() const
{
    ProfileScope;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    activeCamera.lock()->bind();
    sceneRoot->render();
}

void Application::restart()
{
    {
        const auto [framebufferWidth, framebufferHeight] = window->getFramebufferSize();
        onResize(framebufferWidth, framebufferHeight);
    }
}

void Application::onResize(uint32_t width, uint32_t height)
{
    EventQueue::post<event::WindowResized>(width, height);
}
