#include "Application.h"

#include <numbers>
#include <string_view>

#include <Lib/OpenGL.h>
#include <Lib/glm.h>

#include "Assets/AssetLoader.h"
#include "Assets/Model.h"
#include "Assets/Texture.h"
#include "Components/Camera3D.h"
#include "Components/CannonPlayerController.h"
#include "Components/Collider.h"
#include "Components/LightSource.h"
#include "Components/ModelInstance.h"
#include "Components/RigidBody.h"
#include "Components/Transform.h"
#include "Components/Water.h"
#include "Events/EventQueue.h"
#include "Events/Fire.h"
#include "Events/WindowResized.h"
#include "Input.h"
#include "Physics.h"
#include "Singleton.h"
#include "Utils/Color.h"
#include "Utils/Constants.h"
#include "Utils/Log.h"
#include "Utils/Math.h"
#include "Utils/Profiling.h"
#include "Utils/Random.h"

#pragma region model_settings

constexpr const std::string_view SHIP_MODEL = "Ship/Ship.gltf";
constexpr const glm::vec3 SHIP_MODEL_DEFAULT_TRANSLATE = {0.5f, 1.0f, -0.25f};
constexpr const glm::vec3 SHIP_MODEL_DEFAULT_ROTATION = {glm::radians(90.0f), 0.0f, 0.0f};
constexpr const glm::vec3 SHIP_MODEL_DEFAULT_SCALE = 0.5f * glm::vec3{1.0f, 1.0f, 1.0f};
constexpr const component::Collider::AABB SHIP_MODEL_COLLIDER = {
    .half_size = {6.0f, 12.0f, 3.0f},
    .center = {0.0f, 0.0f, 3.0f},
};

constexpr const std::string_view CANNON_STAND_MODEL = "CannonStand/CannonStand.gltf";
constexpr const glm::vec3 CANNON_STAND_MODEl_DEFAULT_TRANSLATE = {};
constexpr const glm::vec3 CANNON_STAND_MODEL_DEFAULT_ROTATION = {glm::radians(90.0f), 0.0f, 0.0f};
constexpr const glm::vec3 CANNON_POSITION = {0.0f, -8.5f, 4.5f};

constexpr const std::string_view CANNON_BARREL_MODEL = "CannonBarrel/CannonBarrel.gltf";
constexpr const glm::vec3 CANNON_BARREL_MODEl_DEFAULT_TRANSLATE = {};
constexpr const glm::vec3 CANNON_BARREL_MODEL_DEFAULT_ROTATION = {glm::radians(8.0f), glm::radians(180.0f), 0.0f};

constexpr const std::string_view CANNONBALL_MODEL = "CannonBall/CannonBall.gltf";
constexpr const glm::vec3 CANNONBALL_MODEL_DEFAULT_TRANSLATE = {};
constexpr const glm::vec3 CANNONBALL_MODEL_DEFAULT_ROTATION = {glm::radians(90.0f), 0.0f, 0.0f};
constexpr const glm::vec3 CANNONBALL_MODEL_DEFAULT_SCALE = 0.4f * glm::vec3{1.0f, 1.0f, 1.0f};
constexpr const float CANNONBALL_MASS = 10.0f;

#pragma endregion model_settings

#pragma region camera_settings

constexpr const double FOV = 45.0;              // °
constexpr const double PERSPECTIVE_NEAR = 0.1;  // m
constexpr const double PERSPECTIVE_FAR = 300.0; // m

constexpr const glm::vec3 CANNON_CAMERA_OFFSET = {1.0f, -4.0f, 1.2f};
constexpr const glm::vec3 CANNONBALL_CAMERA_OFFSET = {0.5f, -2.0f, 0.5f};

static_assert(PERSPECTIVE_FAR > static_cast<double>(WORLD_WIDTH) * std::numbers::sqrt2,
              "Perspective camera far plan not far enough to see the entire map");

#pragma endregion camera_settings

Application::Application() : free_view_override_(false)
{
    ProfileScope;

    Random::initialize();

    window_ = std::make_unique<Window>();
    initializeOpenGL();
    EventQueue::registerCallback<event::WindowResized>([](const event::WindowResized &event) {
        glViewport(0, 0, static_cast<GLsizei>(event.width), static_cast<GLsizei>(event.height));
    });

    Input::initialize(*window_);
    Input::bindKey(Input::Action::ToggleFullScreen, GLFW_KEY_F11);
    Input::bindMouseButton(Input::Action::UIClick, GLFW_MOUSE_BUTTON_1);
    Input::bindKey(Input::Action::ToggleFreeView, GLFW_KEY_ENTER);
    Input::bindKey(Input::Action::CycleRenderingStyles, GLFW_KEY_R);
    Input::bindKey(Input::Action::ToggleDebugMode, GLFW_KEY_F3);
    Input::bindKey(Input::Action::DebugMoveTargetNorth, GLFW_KEY_UP);
    Input::bindKey(Input::Action::DebugMoveTargetEast, GLFW_KEY_RIGHT);
    Input::bindKey(Input::Action::DebugMoveTargetSouth, GLFW_KEY_DOWN);
    Input::bindKey(Input::Action::DebugMoveTargetWest, GLFW_KEY_LEFT);
    Input::bindKey(Input::Action::DebugAimAndFire, GLFW_KEY_F);
    Input::bindKey(Input::Action::CycleCameras, GLFW_KEY_V);

    // Load assets
    AssetLoader::getOrLoadFromFile<asset::Model>(SHIP_MODEL);
    const asset::Model::TextureOverride PLAYER_SHIP_TEXTURE_OVERRIDE = {
        {
            0,
            {
                {asset::Texture::Type::Albedo,
                 AssetLoader::getOrLoadFromFile<asset::Texture>("Ship/SailsRopePlayerAlbedo.png")},
                {asset::Texture::Type::Emissive, nullptr},
            },
        },
    };

    AssetLoader::getOrLoadFromFile<asset::Model>(CANNON_STAND_MODEL);
    AssetLoader::getOrLoadFromFile<asset::Model>(CANNON_BARREL_MODEL);
    AssetLoader::getOrLoadFromFile<asset::Model>(CANNONBALL_MODEL);

#pragma region scene

    scene_root_ = std::make_shared<GameObject>();
    scene_root_->addComponent<component::Transform>();

    // - Free View Camera
    {
        auto perspective_camera = scene_root_->addChild();
        perspective_camera->addComponent<component::Transform>(glm::vec3{5.0f, 5.0f, 5.0f});
        free_view_camera_ = perspective_camera->addComponent<component::Camera3D>(
            component::Camera3D::Perspective{
                .fov = FOV,
                .near = PERSPECTIVE_NEAR,
                .far = PERSPECTIVE_FAR,
            },
            EAST);
        free_view_controls_ = perspective_camera->addComponent<component::FreeViewControls>();
    }

    // - Top View Camera
    {
        auto top_view_camera = scene_root_->addChild();
        top_view_camera->addComponent<component::Transform>(UP * 80.0f - NORTH * 1.0f);
        top_view_camera_ = top_view_camera->addComponent<component::Camera3D>(
            component::Camera3D::Orthographic{
                .scale = 100,
                .near = 10.0,
                .far = 100.0,
            },
            DOWN + NORTH * 0.01f);
    }

    auto sun = scene_root_->addChild();
    sun->addComponent<component::Transform>(UP * 100.0f - NORTH * 30.0f);
    sun->addComponent<component::LightSource>(rgba(252, 231, 165, 1), rgb(255, 255, 255));

    // - Player Ship
    {
        const auto ship_pos = NORTH * 10.0f;

        auto ship = scene_root_->addChild();
        ship->addComponent<component::Transform>(ship_pos);
        ship->addComponent<component::Collider>(SHIP_MODEL_COLLIDER);
        // Physics::addRigidBody(ship->addComponent<component::RigidBody>(10.0f, glm::mat3(1.0f)));

        auto ship_model = ship->addChild();
        ship_model->addComponent<component::Transform>(SHIP_MODEL_DEFAULT_TRANSLATE, SHIP_MODEL_DEFAULT_ROTATION,
                                                       SHIP_MODEL_DEFAULT_SCALE);
        ship_model->addComponent<component::ModelInstance>(AssetLoader::getOrLoadFromFile<asset::Model>(SHIP_MODEL),
                                                           PLAYER_SHIP_TEXTURE_OVERRIDE);

        auto player_target = scene_root_->addChild();
        auto player_target_transform = player_target->addComponent<component::Transform>();
        player_target->addComponent<component::Collider>(component::Collider::AABB{
            .half_size = {0.5f, 0.5f, 0.5f},
            .center = {},
        });

        auto cannon = ship->addChild();
        cannon->addComponent<component::Transform>(CANNON_POSITION);
        player_cannon_id_ = cannon->getId();

        auto cannon_stand_model = cannon->addChild();
        cannon_stand_model->addComponent<component::Transform>(CANNON_STAND_MODEl_DEFAULT_TRANSLATE,
                                                               CANNON_STAND_MODEL_DEFAULT_ROTATION);
        cannon_stand_model->addComponent<component::ModelInstance>(
            AssetLoader::getOrLoadFromFile<asset::Model>(CANNON_STAND_MODEL));

        auto cannon_barrel = cannon->addChild();
        auto cannon_barrel_transform = cannon_barrel->addComponent<component::Transform>();
        cannon_barrel_transform->pointToward(EAST);

        auto cannon_barrel_model = cannon_barrel->addChild();
        cannon_barrel_model->addComponent<component::Transform>(CANNON_BARREL_MODEl_DEFAULT_TRANSLATE,
                                                                CANNON_BARREL_MODEL_DEFAULT_ROTATION);
        cannon_barrel_model->addComponent<component::ModelInstance>(
            AssetLoader::getOrLoadFromFile<asset::Model>(CANNON_BARREL_MODEL));

        auto cannon_camera = cannon->addChild();
        cannon_camera->addComponent<component::Transform>(CANNON_CAMERA_OFFSET);
        cannon_camera_ = cannon_camera->addComponent<component::Camera3D>(
            component::Camera3D::Perspective{
                .fov = FOV,
                .near = PERSPECTIVE_NEAR,
                .far = PERSPECTIVE_FAR,
            },
            EAST);

        cannon->addComponent<component::CannonPlayerController>(cannon_barrel_transform, player_target_transform,
                                                                cannon_camera_);
    }

    // - Enemy 1 Ship
    {
        auto enemy_ship_1 = scene_root_->addChild();
        enemy_ship_1->addComponent<component::Transform>(NORTH * -10.0f);
        enemy_ship_1->addComponent<component::Collider>(SHIP_MODEL_COLLIDER);

        auto enemy_ship_1_model = enemy_ship_1->addChild();
        enemy_ship_1_model->addComponent<component::Transform>(SHIP_MODEL_DEFAULT_TRANSLATE,
                                                               SHIP_MODEL_DEFAULT_ROTATION, SHIP_MODEL_DEFAULT_SCALE);
        enemy_ship_1_model->addComponent<component::ModelInstance>(
            AssetLoader::getOrLoadFromFile<asset::Model>(SHIP_MODEL));
    }

    // - Water
    auto water = scene_root_->addChild();
    water->addComponent<component::Transform>(glm::vec3{}, glm::vec3{}, glm::vec3{1.0f, 1.0f, 1.0f});
    Physics::water_collider_ = water->addComponent<component::Collider>(component::Collider::AABB{
        .half_size = {WORLD_WIDTH / 2.0f, WORLD_WIDTH / 2.0f, 0.5f},
        .center = {0.0f, 0.0f, -0.5f},
    });
    water->addComponent<component::Water>();

#pragma endregion scene

    restart();
    scene_root_->initialize();

    LOG_DEBUG("cannonballs initial velocity: {} m/s", INITIAL_CANNONBALL_VELOCITY);

    const auto water_id = water->getId();
    EventQueue::registerCallback<event::Fire>([this, water_id](const event::Fire &event) {
        if (glm::length(event.initial_velocity) < EPSILON)
        {
            return;
        }

        LOG_DEBUG("fire");

        auto cannonball = scene_root_->addChild();
        std::weak_ptr<GameObject> weak_cannonball = cannonball;
        cannonball->addComponent<component::Transform>(event.position)
            ->pointToward(glm::normalize(event.initial_velocity));
        cannonball->addComponent<component::Collider>(component::Collider::AABB{
            .half_size = {0.5f, 0.5f, 0.5f},
            .center = {},
        });
        auto rigid_body = cannonball->addComponent<component::RigidBody>(CANNONBALL_MASS);
        rigid_body->addCollisionCallback([weak_cannonball, water_id](const GameObjectId id) {
            if (id == water_id)
            {
                weak_cannonball.lock()->detach();
            }
            else
            {
                LOG_WARNING("cannonball collided with game object {} but nothing happend", id);
            }

            // check_camera TODO
        });
        rigid_body->setVelocity(event.initial_velocity);

        auto cannonball_model = cannonball->addChild();
        cannonball_model->addComponent<component::Transform>(
            CANNONBALL_MODEL_DEFAULT_TRANSLATE, CANNONBALL_MODEL_DEFAULT_ROTATION, CANNONBALL_MODEL_DEFAULT_SCALE);
        cannonball_model->addComponent<component::ModelInstance>(
            AssetLoader::getOrLoadFromFile<asset::Model>(CANNONBALL_MODEL));

        if (event.shooter == player_cannon_id_)
        {
            auto cannonball_camera = cannonball->addChild();
            cannonball_camera->addComponent<component::Transform>(CANNONBALL_CAMERA_OFFSET);
            // last_cannonball_camera_ = {cannonball_camera->addComponent<component::Camera3D>(
            //     component::Camera3D::Perspective{
            //         .fov = FOV,
            //         .near = PERSPECTIVE_NEAR,
            //         .far = PERSPECTIVE_FAR,
            //     },
            //     event.initial_velocity - UP * glm::dot(UP, event.initial_velocity))}; //  TODO
        }

        cannonball->initialize();
    });

    Singleton::game_loaded = true;

    main_view_ = View::Top;
    Singleton::view = main_view_;
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

    Input::update();

    if (Input::getState(Input::Action::ToggleFullScreen) == Input::State::JustReleased)
    {
        window_->toggleFullScreen();
    }
    if (Input::getState(Input::Action::ToggleFreeView) == Input::State::JustReleased)
    {
        if (!free_view_override_)
        {
            free_view_override_ = true;
            free_view_controls_.lock()->active = true;
            window_->captureMouse();
            LOG_INFO("free view mode enabled");
        }
        else
        {
            free_view_override_ = false;
            free_view_controls_.lock()->active = false;
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
    if (Input::getState(Input::Action::CycleCameras) == Input::State::JustReleased)
    {
        switch (main_view_)
        {
        case View::FreeCamera:
            LOG_WARNING("This should not be reachable: main_view_ should never be View::FreeCamera");
            break;
        case View::Top:
            main_view_ = last_cannonball_camera_.has_value() ? View::CannonBall : View::Cannon;
            break;
        case View::Cannon:
            [[fallthrough]];
        case View::CannonBall:
            main_view_ = View::Top;
            break;
        }
    }

    // Resolve active camera
    if (free_view_override_)
    {
        Singleton::active_camera = free_view_camera_;
        Singleton::view = View::FreeCamera;
    }
    else
    {
        Singleton::view = main_view_;
        switch (main_view_)
        {
        case View::FreeCamera:
            LOG_WARNING("This should not be reachable: main_view_ should never be View::FreeCamera");
            break;
        case View::Top:
            Singleton::active_camera = top_view_camera_;
            break;
        case View::Cannon:
            Singleton::active_camera = cannon_camera_;
            break;
        case View::CannonBall:
            Singleton::active_camera = last_cannonball_camera_.value();
            break;
        }
    }

    EventQueue::processAll();

    scene_root_->update(delta_time);
    Physics::update(delta_time);
}

void Application::render() const
{
    ProfileScope;

    constexpr const auto SKY_COLOR = rgb(193, 234, 255);
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
