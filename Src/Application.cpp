#include "Application.h"

#include <numbers>
#include <string_view>

#include <Lib/OpenGL.h>
#include <Lib/glm.h>

#include "Components/Animation.h"
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
#include "Resources/Model.h"
#include "Resources/ResourceLoader.h"
#include "Resources/Texture.h"
#include "Utils/Color.h"
#include "Utils/Constants.h"
#include "Utils/Log.h"
#include "Utils/Math.h"
#include "Utils/MeshPrimitives.h"
#include "Utils/Profiling.h"
#include "Utils/Random.h"

// #define DEBUG_SCENE

#pragma region model_settings

constexpr const std::string_view SHIP_MODEL = "Ship/Ship.gltf";
constexpr const glm::vec3 SHIP_MODEL_TRANSLATION = -0.5f * MODEL_RIGHT;
constexpr const glm::vec3 SHIP_MODEL_ROTATION = {glm::radians(90.0f), 0.0f, glm::radians(180.0f)};
constexpr const glm::vec3 SHIP_MODEL_SCALE = 0.5f * ONE;
constexpr const component::Collider::AABB SHIP_MODEL_COLLIDER = {
    .half_size = 6.0f * MODEL_RIGHT + 12.0f * MODEL_FORWARD + 3.0f * MODEL_UP,
    .center = 3.0f * MODEL_UP,
};

constexpr const std::string_view CANNON_STAND_MODEL = "CannonStand/CannonStand.gltf";
constexpr const glm::vec3 CANNON_STAND_MODEL_TRANSLATION = {0.0f, 0.0f, 1.0f};
constexpr const glm::vec3 CANNON_STAND_MODEL_ROTATION = {glm::radians(90.0f), 0.0f, glm::radians(180.0f)};
constexpr const glm::vec3 CANNON_STAND_MODEL_SCALE = ONE;

constexpr const std::string_view CANNON_BARREL_MODEL = "CannonBarrel/CannonBarrel.gltf";
constexpr const glm::vec3 CANNON_BARREL_MODEL_TRANSLATION = ZERO;
constexpr const glm::vec3 CANNON_BARREL_MODEL_ROTATION = {glm::radians(99.0f), 0.0f, glm::radians(180.0f)};
constexpr const glm::vec3 CANNON_BARREL_MODEL_SCALE = ONE;

constexpr const glm::vec3 CANNON_POSITION_IN_SHIP = 8.5f * MODEL_FORWARD + 3.7f * MODEL_UP;
constexpr const glm::vec3 CANNON_BARREL_POSITION_IN_CANNON = 1.0f * MODEL_UP;
constexpr const glm::vec3 CANNON_BARREL_ROTATION_IN_CANNON = {glm::radians(-90.0f), glm::radians(90.0f), 0.0f};

constexpr const std::string_view CANNON_BALL_MODEL = "CannonBall/CannonBall.gltf";
constexpr const glm::vec3 CANNON_BALL_MODEL_TRANSLATION = ZERO;
constexpr const glm::vec3 CANNON_BALL_MODEL_ROTATION = {0.0f, 0.0f, glm::radians(180.0f)};
constexpr const glm::vec3 CANNON_BALL_MODEL_SCALE = 0.4f * ONE;
constexpr const float CANNON_BALL_MASS = 10.0f;

constexpr const std::string_view RADAR_CYLINDER_MODEL = "RadarCylinder";
constexpr const float RADAR_CYLINDER_HEIGHT = 1.0f;
constexpr const float RADAR_CYLINDER_RAIDUS = 0.2f;
constexpr const size_t RADAR_CYLINDER_RESOLUTION = 12;
constexpr const Color RADAR_CYLINDER_COLOR = rgba(72, 43, 5, 1);
constexpr const glm::vec3 RADAR_CYLINDER_MODEL_TRANSLATION = MODEL_UP * 0.5f;

constexpr const std::string_view RADAR_CONE_MODEL = "RadarCone";
constexpr const float RADAR_CONE_HEIGHT = 1.0f;
constexpr const float RADAR_CONE_RAIDUS = 0.3f;
constexpr const size_t RADAR_CONE_RESOLUTION = 12;
constexpr const Color RADAR_CONE_COLOR = rgba(255, 0, 0, 1);
constexpr const glm::vec3 RADAR_CONE_MODEL_POSITION = MODEL_UP * 0.9f;
constexpr const glm::vec3 RADAR_CONE_MODEL_ROTATION = {glm::radians(90.0f), 0.0f, 0.0f};

constexpr const glm::vec3 RADAR_POSITION = 1.5f * MODEL_LEFT + 9.0f * MODEL_BACKWARD + 6.05f * MODEL_UP;

const component::Animation::Callback RADAR_ANIMATION = [](float delta_time,
                                                          std::shared_ptr<component::Transform> transform) {
    constexpr const float ROTATION_SPEED = 2.0f * std::numbers::pi_v<float> / 3.0f;
    transform->rotate(ROTATION_SPEED * delta_time, UP);
};

#pragma endregion model_settings

#pragma region camera_settings

constexpr const double FOV = 45.0;              // °
constexpr const double PERSPECTIVE_NEAR = 0.1;  // m
constexpr const double PERSPECTIVE_FAR = 300.0; // m

constexpr const glm::vec3 CANNON_CAMERA_OFFSET = {1.0f, -4.0f, 1.2f};
constexpr const glm::vec3 CANNON_BALL_CAMERA_OFFSET = {0.5f, 0.5f, 2.0f};

static_assert(PERSPECTIVE_FAR > static_cast<double>(WORLD_WIDTH) * std::numbers::sqrt2,
              "Perspective camera far plan not far enough to see the entire map");

#pragma endregion camera_settings

#pragma region ship_definition

#define CREATE_SHIP(prefix, position, texture_override)                                                                \
    auto prefix##_ship = scene_root_->addChild();                                                                      \
    prefix##_ship->addComponent<component::Transform>(position);                                                       \
    prefix##_ship->addComponent<component::Collider>(SHIP_MODEL_COLLIDER);                                             \
                                                                                                                       \
    /* ship model */                                                                                                   \
    auto prefix##_ship_model = prefix##_ship->addChild();                                                              \
    prefix##_ship_model->addComponent<component::Transform>(SHIP_MODEL_TRANSLATION, SHIP_MODEL_ROTATION,               \
                                                            SHIP_MODEL_SCALE);                                         \
    prefix##_ship_model->addComponent<component::ModelInstance>(ResourceLoader::getAsset<resource::Model>(SHIP_MODEL), \
                                                                texture_override);                                     \
                                                                                                                       \
    /* target (visible in debug mode) */                                                                               \
    auto prefix##_target = scene_root_->addChild();                                                                    \
    auto prefix##_target_transform = prefix##_target->addComponent<component::Transform>();                            \
    prefix##_target->addComponent<component::Collider>(component::Collider::AABB{                                      \
        .half_size = {0.5f, 0.5f, 0.5f},                                                                               \
        .center = {},                                                                                                  \
    });                                                                                                                \
                                                                                                                       \
    /* cannon */                                                                                                       \
    auto prefix##_cannon = prefix##_ship->addChild();                                                                  \
    prefix##_cannon->addComponent<component::Transform>(CANNON_POSITION_IN_SHIP);                                      \
                                                                                                                       \
    /* - stand model */                                                                                                \
    auto prefix##_cannon_stand_model = prefix##_cannon->addChild();                                                    \
    prefix##_cannon_stand_model->addComponent<component::Transform>(                                                   \
        CANNON_STAND_MODEL_TRANSLATION, CANNON_STAND_MODEL_ROTATION, CANNON_STAND_MODEL_SCALE);                        \
    prefix##_cannon_stand_model->addComponent<component::ModelInstance>(                                               \
        ResourceLoader::getAsset<resource::Model>(CANNON_STAND_MODEL));                                                \
                                                                                                                       \
    /* - barrel container */                                                                                           \
    auto prefix##_barrel_container = prefix##_cannon->addChild();                                                      \
    prefix##_barrel_container->addComponent<component::Transform>(CANNON_BARREL_POSITION_IN_CANNON,                    \
                                                                  CANNON_BARREL_ROTATION_IN_CANNON);                   \
                                                                                                                       \
    /* - barrel */                                                                                                     \
    auto prefix##_cannon_barrel = prefix##_barrel_container->addChild();                                               \
    auto prefix##_cannon_barrel_transform = prefix##_cannon_barrel->addComponent<component::Transform>();              \
    prefix##_cannon_barrel_transform->pointToward(EAST);                                                               \
                                                                                                                       \
    /* - barrel model */                                                                                               \
    auto prefix##_cannon_barrel_model = prefix##_cannon_barrel->addChild();                                            \
    prefix##_cannon_barrel_model->addComponent<component::Transform>(                                                  \
        CANNON_BARREL_MODEL_TRANSLATION, CANNON_BARREL_MODEL_ROTATION, CANNON_BARREL_MODEL_SCALE);                     \
    prefix##_cannon_barrel_model->addComponent<component::ModelInstance>(                                              \
        ResourceLoader::getAsset<resource::Model>(CANNON_BARREL_MODEL));                                               \
                                                                                                                       \
    /* radar */                                                                                                        \
    auto prefix##_radar = prefix##_ship->addChild();                                                                   \
    prefix##_radar->addComponent<component::Transform>(RADAR_POSITION);                                                \
    prefix##_radar->addComponent<component::Animation>(RADAR_ANIMATION);                                               \
                                                                                                                       \
    /* - cylinder */                                                                                                   \
    auto prefix##_radar_cylinder = prefix##_radar->addChild();                                                         \
    prefix##_radar_cylinder->addComponent<component::Transform>(RADAR_CYLINDER_MODEL_TRANSLATION);                     \
    prefix##_radar_cylinder->addComponent<component::ModelInstance>(                                                   \
        ResourceLoader::get<resource::Model>(std::string(RADAR_CYLINDER_MODEL)));                                      \
                                                                                                                       \
    /* - cone */                                                                                                       \
    auto prefix##_radar_cone = prefix##_radar->addChild();                                                             \
    prefix##_radar_cone->addComponent<component::Transform>(RADAR_CONE_MODEL_POSITION, RADAR_CONE_MODEL_ROTATION);     \
    prefix##_radar_cone->addComponent<component::ModelInstance>(                                                       \
        ResourceLoader::get<resource::Model>(std::string(RADAR_CONE_MODEL)))

#pragma endregion ship_definition

Application::Application() : free_view_override_(false), physics_(true)
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
    Input::bindKey(Input::Action::TogglePhysics, GLFW_KEY_P);

    // Load resources
    ResourceLoader::getAsset<resource::Model>(SHIP_MODEL);
    const resource::Model::TextureOverride PLAYER_SHIP_TEXTURE_OVERRIDE = {
        {
            0,
            {
                {resource::Texture::Type::Albedo,
                 ResourceLoader::getAsset<resource::Texture>("Ship/SailsRopePlayerAlbedo.png")},
                {resource::Texture::Type::Emissive, nullptr},
            },
        },
    };

    ResourceLoader::getAsset<resource::Model>(CANNON_STAND_MODEL);
    ResourceLoader::getAsset<resource::Model>(CANNON_BARREL_MODEL);
    ResourceLoader::getAsset<resource::Model>(CANNON_BALL_MODEL);

    ResourceLoader::load<resource::Model>(
        std::string(RADAR_CYLINDER_MODEL),
        generateCylinder(RADAR_CYLINDER_HEIGHT, RADAR_CYLINDER_RAIDUS, RADAR_CYLINDER_RESOLUTION),
        RADAR_CYLINDER_COLOR);
    ResourceLoader::load<resource::Model>(std::string(RADAR_CONE_MODEL),
                                          generateCone(RADAR_CONE_HEIGHT, RADAR_CONE_RAIDUS, RADAR_CONE_RESOLUTION),
                                          RADAR_CONE_COLOR);

#pragma region scene

    scene_root_ = std::make_shared<GameObject>();
    scene_root_->addComponent<component::Transform>();

    // - Free View Camera
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

    // - Top View Camera
    auto top_view_camera = scene_root_->addChild();
    top_view_camera->addComponent<component::Transform>(UP * 80.0f - NORTH * 1.0f);
    top_view_camera_ = top_view_camera->addComponent<component::Camera3D>(
        component::Camera3D::Orthographic{
            .scale = 100,
            .near = 10.0,
            .far = 100.0,
        },
        glm::normalize(DOWN + NORTH * 0.01f));

    // - Sun
    auto sun = scene_root_->addChild();
    sun->addComponent<component::Transform>(UP * 100.0f - NORTH * 30.0f);
    sun->addComponent<component::LightSource>(rgba(252, 231, 165, 1), rgb(255, 255, 255));

#if defined(DEBUG_SCENE)

    // Ship
    auto ship = scene_root_->addChild();
    ship->addComponent<component::Transform>();

    auto ship_model = ship->addChild();
    ship_model->addComponent<component::Transform>(SHIP_MODEL_TRANSLATION, SHIP_MODEL_ROTATION, SHIP_MODEL_SCALE);
    ship_model->addComponent<component::ModelInstance>(ResourceLoader::getAsset<resource::Model>(SHIP_MODEL));

    // Cannon Stand
    auto cannon_stand = scene_root_->addChild();
    cannon_stand->addComponent<component::Transform>(EAST * 10.0f);

    auto cannon_stand_model = cannon_stand->addChild();
    cannon_stand_model->addComponent<component::Transform>(CANNON_STAND_MODEL_TRANSLATION, CANNON_STAND_MODEL_ROTATION,
                                                           CANNON_STAND_MODEL_SCALE);
    cannon_stand_model->addComponent<component::ModelInstance>(
        ResourceLoader::getAsset<resource::Model>(CANNON_STAND_MODEL));

    // Cannon Barrel
    auto cannon_barrel = scene_root_->addChild();
    cannon_barrel->addComponent<component::Transform>(EAST * 15.0f);

    auto cannon_barrel_model = cannon_barrel->addChild();
    cannon_barrel_model->addComponent<component::Transform>(CANNON_BARREL_MODEL_TRANSLATION,
                                                            CANNON_BARREL_MODEL_ROTATION, CANNON_BARREL_MODEL_SCALE);
    cannon_barrel_model->addComponent<component::ModelInstance>(
        ResourceLoader::getAsset<resource::Model>(CANNON_BARREL_MODEL));

    // Cannon Ball
    auto cannon_ball = scene_root_->addChild();
    cannon_ball->addComponent<component::Transform>(EAST * 20.0f);

    auto cannon_ball_model = cannon_ball->addChild();
    cannon_ball_model->addComponent<component::Transform>(CANNON_BALL_MODEL_TRANSLATION, CANNON_BALL_MODEL_ROTATION,
                                                          CANNON_BALL_MODEL_SCALE);
    cannon_ball_model->addComponent<component::ModelInstance>(
        ResourceLoader::getAsset<resource::Model>(CANNON_BALL_MODEL));

#else

    // - Player
    const auto player_ship_position = EAST * 10.0f;

    CREATE_SHIP(player, player_ship_position, PLAYER_SHIP_TEXTURE_OVERRIDE);

    player_cannon_id_ = player_cannon->getId();

    auto cannon_camera = player_cannon->addChild();
    cannon_camera->addComponent<component::Transform>(CANNON_CAMERA_OFFSET);
    cannon_camera_ = cannon_camera->addComponent<component::Camera3D>(
        component::Camera3D::Perspective{
            .fov = FOV,
            .near = PERSPECTIVE_NEAR,
            .far = PERSPECTIVE_FAR,
        },
        EAST);

    player_cannon->addComponent<component::CannonPlayerController>(player_cannon_barrel_transform,
                                                                   player_target_transform, cannon_camera_);

    // - Enemy 1
    const auto enemy_ship_position = EAST * -10.0f;

    CREATE_SHIP(enemy, enemy_ship_position, resource::Model::TextureOverride{});

#endif

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

    LOG_DEBUG("cannonballs initial velocity: {} m/s", INITIAL_CANNON_BALL_VELOCITY);

#if !defined(DEBUG_SCENE)
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
        auto rigid_body = cannonball->addComponent<component::RigidBody>(CANNON_BALL_MASS);
        rigid_body->addCollisionCallback([this, weak_cannonball, water_id](const GameObjectId id) {
            if (last_cannonball_camera_.has_value() &&
                weak_cannonball.lock()->getId() ==
                    last_cannonball_camera_.value().lock()->getOwner()->getParent().value()->getId())
            {
                last_cannonball_camera_ = std::nullopt;
                if (main_view_ == View::CannonBall)
                {
                    main_view_ = View::Cannon;
                    updateActiveView();
                }
            }

            if (id == water_id)
            {
                weak_cannonball.lock()->detach();
            }
            else
            {
                LOG_WARNING("cannonball collided with game object {} but nothing happend", id);
            }
        });
        rigid_body->setVelocity(event.initial_velocity);

        auto cannonball_model = cannonball->addChild();
        cannonball_model->addComponent<component::Transform>(CANNON_BALL_MODEL_TRANSLATION, CANNON_BALL_MODEL_ROTATION,
                                                             CANNON_BALL_MODEL_SCALE);
        cannonball_model->addComponent<component::ModelInstance>(
            ResourceLoader::getAsset<resource::Model>(CANNON_BALL_MODEL));

        if (event.shooter == player_cannon_id_)
        {
            auto cannonball_camera = cannonball->addChild();
            cannonball_camera->addComponent<component::Transform>(CANNON_BALL_CAMERA_OFFSET);
            last_cannonball_camera_ = {cannonball_camera->addComponent<component::Camera3D>(
                component::Camera3D::Perspective{
                    .fov = FOV,
                    .near = PERSPECTIVE_NEAR,
                    .far = PERSPECTIVE_FAR,
                },
                NORTH)};

            if (main_view_ == View::Cannon)
            {
                main_view_ = View::CannonBall;
                updateActiveView();
            }
        }

        cannonball->initialize();

        if (event.shooter == player_cannon_id_)
        {
            last_cannonball_camera_.value().lock()->lookToward(glm::normalize(event.initial_velocity));
        }
    });
#endif

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

void Application::updateActiveView()
{
    if (free_view_override_)
    {
        Singleton::active_camera = free_view_camera_;
        Singleton::view = View::FreeCamera;
        return;
    }

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
    if (Input::getState(Input::Action::TogglePhysics) == Input::State::JustReleased)
    {
        physics_ = !physics_;
    }

    updateActiveView();

    EventQueue::processAll();

    scene_root_->update(delta_time);

    if (physics_)
    {
        Physics::update(delta_time);
    }
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
