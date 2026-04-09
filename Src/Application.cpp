#include "Application.h"

#include <GLFW/glfw3.h>

#include "Assets/AssetLoader.h"
#include "Assets/Mesh.h"
#include "Components/Camera3D.h"
#include "Components/LightSource.h"
#include "Components/MeshInstance.h"
#include "Components/Transform.h"
#include "Events/EventQueue.h"
#include "Events/WindowResized.h"
#include "Input.h"
#include "Singleton.h"
#include "Utils/Color.h"
#include "Utils/Profiling.h"
#include "Utils/Random.h"

Application::Application()
{
    ProfileScope;

    Random::initialize();
    window = std::make_unique<Window>();

    initializeOpenGL();

    Input::initialize(*window);
    Input::bindKey(Input::Action::ToggleFullScreen, GLFW_KEY_F11);
    Input::bindMouseButton(Input::Action::UIClick, GLFW_MOUSE_BUTTON_1);
    Input::bindKey(Input::Action::ToggleFreeView, GLFW_KEY_ENTER);

    AssetLoader::load<Mesh>("Models/Ship.obj");

    EventQueue::registerCallback<event::WindowResized>([](const event::WindowResized &event) {
        glViewport(0, 0, static_cast<GLsizei>(event.width), static_cast<GLsizei>(event.height));
    });

    sceneRoot = std::make_shared<GameObject>();

    auto perspectiveCamera = sceneRoot->addChild();
    perspectiveCamera->addComponent<component::Transform>(glm::vec3{2.0f, 2.0f, 2.0f});
    Singleton::activeCamera = perspectiveCamera->addComponent<component::Camera3D>(component::Camera3D::Perspective{
        .fov = 45.0f,
        .near = 0.1f,
        .far = 100.0f,
        .lookAt = {0.0f, 0.0f, 0.0f},
    });
    freeViewControls = perspectiveCamera->addComponent<component::FreeViewControls>();
    perspectiveCamera->addComponent<component::LightSource>(rgba(255, 255, 255, 1), rgb(225, 225, 225));

    auto ship = sceneRoot->addChild();
    ship->addComponent<component::Transform>();
    ship->addComponent<component::MeshInstance>(AssetLoader::get<Mesh>("Models/Ship.obj"));

    restart();

    sceneRoot->initialize();
}

void Application::initializeOpenGL()
{
    // Transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Depth Test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Back Faces Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Light
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, reinterpret_cast<const GLfloat *>(&component::LightSource::AMBIENT_COLOR));
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
    if (Input::getState(Input::Action::ToggleFreeView) == Input::State::JustReleased)
    {
        auto &freeViewControlsEnabled = freeViewControls.lock()->active;

        freeViewControlsEnabled = !freeViewControlsEnabled;
        if (freeViewControlsEnabled)
        {
            window->captureMouse();
            LOG_INFO("free view mode enabled");
        }
        else
        {
            window->releaseMouse();
            LOG_INFO("free view mode disabled");
        }
    }

    sceneRoot->update(deltaTime);
}

void Application::render() const
{
    ProfileScope;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Singleton::activeCamera.lock()->bind();
    sceneRoot->render();
}

void Application::restart()
{
    const auto [framebufferWidth, framebufferHeight] = window->getFramebufferSize();
    EventQueue::post<event::WindowResized>(framebufferWidth, framebufferHeight);
}
