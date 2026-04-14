#include "Application.h"

#include <Lib/OpenGL.h>

#include "Assets/AssetLoader.h"
#include "Assets/Model.h"
#include "Components/Camera3D.h"
#include "Components/Collider.h"
#include "Components/LightSource.h"
#include "Components/ModelInstance.h"
#include "Components/Transform.h"
#include "Components/Water.h"
#include "Events/EventQueue.h"
#include "Events/WindowResized.h"
#include "Input.h"
#include "Physics.h"
#include "Singleton.h"
#include "Utils/Color.h"
#include "Utils/Constants.h"
#include "Utils/Log.h"
#include "Utils/Profiling.h"
#include "Utils/Random.h"

Application::Application()
{
    ProfileScope;

    Random::initialize();
    window_ = std::make_unique<Window>();

    initializeOpenGL();

    Input::initialize(*window_);
    Input::bindKey(Input::Action::ToggleFullScreen, GLFW_KEY_F11);
    Input::bindMouseButton(Input::Action::UIClick, GLFW_MOUSE_BUTTON_1);
    Input::bindKey(Input::Action::ToggleFreeView, GLFW_KEY_ENTER);
    Input::bindKey(Input::Action::CycleRenderingStyles, GLFW_KEY_R);
    Input::bindKey(Input::Action::ToggleDebugMode, GLFW_KEY_F3);
    Input::bindKey(Input::Action::ArrowLeft, GLFW_KEY_LEFT);
    Input::bindKey(Input::Action::ArrowRight, GLFW_KEY_RIGHT);

    // Load assets
    constexpr const std::string_view SHIP_MODEL = "Models/Ship/Ship.gltf";
    AssetLoader::get<asset::Model>(SHIP_MODEL);

    EventQueue::registerCallback<event::WindowResized>([](const event::WindowResized &event) {
        glViewport(0, 0, static_cast<GLsizei>(event.width), static_cast<GLsizei>(event.height));
    });

    scene_root_ = std::make_shared<GameObject>();
    scene_root_->addComponent<component::Transform>();

    auto perspective_camera = scene_root_->addChild();
    perspective_camera->addComponent<component::Transform>(glm::vec3{5.0f, 5.0f, 5.0f});
    free_view_camera_ = perspective_camera->addComponent<component::Camera3D>(
        component::Camera3D::Perspective{
            .fov = 45.0,
            .near = 0.1,
            .far = 100.0,
        },
        glm::vec3{0.0f, 0.0f, 0.0f});
    free_view_controls_ = perspective_camera->addComponent<component::FreeViewControls>();

    auto top_view_camera = scene_root_->addChild();
    top_view_camera->addComponent<component::Transform>(UP * 80.0f - NORTH * 1.0f);
    Singleton::main_camera = top_view_camera->addComponent<component::Camera3D>(
        component::Camera3D::Orthographic{
            .scale = 100,
            .near = 10.0,
            .far = 100.0,
        },
        glm::vec3{0.0f, 0.0f, 0.0f});

    auto sun = scene_root_->addChild();
    sun->addComponent<component::Transform>(UP * 100.0f - NORTH * 30.0f);
    sun->addComponent<component::LightSource>(rgba(252, 231, 165, 1), rgb(255, 255, 255));

    auto ship = scene_root_->addChild();
    ship->addComponent<component::Transform>(glm::vec3{10.0f, 5.0f, 3.0f});
    ship->addComponent<component::Collider>(component::Collider::AABB{
        .half_size = {6.0f, 12.0f, 3.0f},
        .center = {0.0f, 0.0f, 3.0f},
    });

    auto ship_model = ship->addChild();
    ship_model->addComponent<component::Transform>(
        glm::vec3{0.5f, 1.0f, -0.25f}, glm::vec3{glm::radians(90.0f), 0.0f, 0.0f}, 0.5f * glm::vec3{1.0f, 1.0f, 1.0f});
    ship_model->addComponent<component::ModelInstance>(AssetLoader::get<asset::Model>(SHIP_MODEL));

    auto water = scene_root_->addChild();
    water->addComponent<component::Transform>(glm::vec3{}, glm::vec3{}, glm::vec3{160.0f, 160.0f, 1.0f});
    water->addComponent<component::Water>();

    restart();

    scene_root_->initialize();

    Singleton::game_loaded = true;
    Singleton::active_camera = Singleton::main_camera;
}

void Application::initializeOpenGL()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
    while (!window_->shouldClose())
    {
        const float delta_time = clock_.tick();
        if (delta_time > 1.0f)
        {
            continue;
        }

        update(delta_time);
        render();

        window_->endFrame();

        ProfilingEndFrame;
    }
}

void Application::update(float delta_time)
{
    ProfileScope;

    EventQueue::processAll();

    Input::update();

    if (Input::getState(Input::Action::ToggleFullScreen) == Input::State::JustReleased)
    {
        window_->toggleFullScreen();
    }
    if (Input::getState(Input::Action::ToggleFreeView) == Input::State::JustReleased)
    {
        auto &free_view_controls_enabled = free_view_controls_.lock()->active;

        free_view_controls_enabled = !free_view_controls_enabled;
        if (free_view_controls_enabled)
        {
            Singleton::active_camera = free_view_camera_;
            window_->captureMouse();
            LOG_INFO("free view mode enabled");
        }
        else
        {
            Singleton::active_camera = Singleton::main_camera;
            window_->releaseMouse();
            LOG_INFO("free view mode disabled");
        }
    }
    if (Input::getState(Input::Action::CycleRenderingStyles) == Input::State::JustReleased)
    {
        switch (Singleton::rendering_style)
        {
        case RenderingStyle::OpaquePolygon: {
            LOG_INFO("switched to wireframe rendering");
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            Singleton::rendering_style = RenderingStyle::Wireframe;
        }
        break;
        case RenderingStyle::Wireframe: {
            LOG_INFO("switched to opaque polygon rendering");
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            Singleton::rendering_style = RenderingStyle::OpaquePolygon;
        }
        break;
        }
    }
    if (Input::getState(Input::Action::ToggleDebugMode) == Input::State::JustReleased)
    {
        Singleton::debug = !Singleton::debug;
        if (Singleton::debug)
        {
            LOG_INFO("debug mode enabled");
        }
        else
        {
            LOG_INFO("debug mode disabled");
        }
    }

    scene_root_->update(delta_time);
    Physics::update(delta_time);
}

void Application::render() const
{
    ProfileScope;

    constexpr const auto SKY_COLOR = rgba(193, 234, 255, 1);
    glClearColor(_v4(SKY_COLOR));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Singleton::active_camera.lock()->bind();
    scene_root_->render();
}

void Application::restart()
{
    const auto [framebuffer_width, framebuffer_height] = window_->getFramebufferSize();
    EventQueue::post<event::WindowResized>(framebuffer_width, framebuffer_height);
}
