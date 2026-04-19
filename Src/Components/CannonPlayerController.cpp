#include "Components/CannonPlayerController.h"

#include <cmath>

#include <Lib/OpenGL.h>
#include <Lib/glfw.h>

#include "Events/EventQueue.h"
#include "Events/Fire.h"
#include "GameObject.h" // IWYU pragma: keep
#include "Input.h"
#include "Physics.h"
#include "Singleton.h"
#include "Utils/Color.h"
#include "Utils/Constants.h"
#include "Utils/Log.h"
#include "Utils/Math.h"
#include "Utils/Profiling.h"

using namespace component;

CannonPlayerController::CannonPlayerController(std::weak_ptr<Transform> cannon_barrel_transform,
                                               std::weak_ptr<Transform> target_transform,
                                               std::weak_ptr<Camera3D> camera)
    : barrel_transform_(cannon_barrel_transform), target_transform_(target_transform), camera_(camera), aiming_(false)
{
    Input::bindMouseButton(Input::Action::AimAndFire, GLFW_MOUSE_BUTTON_LEFT);
    Input::bindMouseButton(Input::Action::CancelFire, GLFW_MOUSE_BUTTON_RIGHT);
}

void CannonPlayerController::initialize()
{
    GET_COMPONENT(Transform, transform_, CannonPlayerController);
}

glm::vec3 CannonPlayerController::getShootingInitialVelocity(const glm::vec3 &target) const
{
    /// c.f. Ballistic.pdf
    const auto position = glm::vec3(barrel_transform_.lock()->resolve()[3]);

    const auto delta = target - position;

    const float a = GRAVITY * GRAVITY / 4.0f;
    const float b = glm::dot(delta, UP) * GRAVITY + INITIAL_CANNON_BALL_VELOCITY * INITIAL_CANNON_BALL_VELOCITY;
    const float c = glm::dot(delta, delta);

    const float disc = b * b - 4.0f * a * c;
    if (disc < 0.0f)
    {
        LOG_WARNING("target out of range");
        return {};
    }

    // const float T = (b + std::sqrtf(disc)) / (2.0f * a); // T_plus
    const float T = (b - std::sqrtf(disc)) / (2.0f * a); // T_minus
    const float t = std::sqrtf(T);
    return delta / t + 0.5f * GRAVITY * UP * t;
}

void CannonPlayerController::update(float delta_time)
{
    ProfileScope;

    constexpr const float DEBUG_TARGET_SPEED = 15.0f; // m/s

    auto transform = transform_.lock();
    auto barrel_transform = barrel_transform_.lock();

    if (Input::getState(Singleton::view != View::Top ? Input::Action::DebugAimAndFire : Input::Action::AimAndFire) ==
        Input::State::JustPressed)
    {
        aiming_ = true;
    }

    if (Input::getState(Input::Action::CancelFire) == Input::State::JustReleased)
    {
        aiming_ = false;
        LOG_DEBUG("fire canceled");
    }

    bool target_moved = false;
    glm::vec3 target;
    auto resolved_transform = transform->resolve();
    const auto position = glm::vec3(resolved_transform[3]);

    glm::vec3 target_motion{};
    if (Input::isPressed(Input::Action::DebugMoveTargetNorth))
    {
        target_motion += NORTH;
    }
    if (Input::isPressed(Input::Action::DebugMoveTargetEast))
    {
        target_motion += EAST;
    }
    if (Input::isPressed(Input::Action::DebugMoveTargetSouth))
    {
        target_motion += SOUTH;
    }
    if (Input::isPressed(Input::Action::DebugMoveTargetWest))
    {
        target_motion += WEST;
    }

    if (aiming_ && Singleton::view == View::Top)
    {
        target = Singleton::active_camera.lock()->screenToWorld(Input::getMousePosition());
        target_moved = true;
    }
    else if (glm::length(target_motion) > EPSILON)
    {
        const auto initial_target_position = glm::vec3(target_transform_.lock()->resolve()[3]);
        target = initial_target_position + glm::normalize(target_motion) * DEBUG_TARGET_SPEED * delta_time;
        target_moved = true;
    }

    const auto target_along_north = glm::dot(target, NORTH);
    const auto target_along_east = glm::dot(target, EAST);
    if (target_along_north < -WORLD_WIDTH / 2.0f || WORLD_WIDTH / 2.0f < target_along_north ||
        target_along_east < -WORLD_WIDTH / 2.0f || WORLD_WIDTH / 2.0f < target_along_east)
    {
        aiming_ = false;
        target_moved = false;
    }

    if (target_moved)
    {
        target -= glm::dot(target, UP) * UP;
        target_transform_.lock()->setPosition(target);

        // Cannon stand
        const auto planar_position = position - glm::dot(position, UP) * UP;
        const auto target_direction = glm::normalize(target - planar_position);

        const auto cos_target_angle = glm::dot(NORTH, target_direction);
        const auto sin_target_angle = glm::dot(glm::cross(NORTH, target_direction), UP);
        const auto target_angle = std::atan2(sin_target_angle, cos_target_angle);

        const auto current_rotation_matrix = glm::mat3(resolved_transform);
        const auto current_rotation = glm::quat_cast(current_rotation_matrix);
        const auto current_angle = angleAroundAxis(current_rotation, UP);

        transform->rotate(target_angle - current_angle, UP);

        // Cannon barrel
        cannonball_initial_velocity_ = getShootingInitialVelocity(target);
        resolved_transform = transform->resolve();
        const auto parent_rot = glm::quat_cast(glm::mat3(resolved_transform));
        const auto local_direction = glm::inverse(parent_rot) * cannonball_initial_velocity_;

        barrel_transform->pointToward(glm::normalize(local_direction));

        // Camera
        const auto planar_velocity = cannonball_initial_velocity_ - UP * glm::dot(UP, cannonball_initial_velocity_);
        if (glm::length(planar_velocity) > EPSILON)
        {
            camera_.lock()->lookToward(glm::normalize(planar_velocity));
        }
    }

    if (Input::getState(Singleton::view != View::Top ? Input::Action::DebugAimAndFire : Input::Action::AimAndFire) ==
        Input::State::JustReleased)
    {
        if (aiming_)
        {
            EventQueue::post<event::Fire>(glm::vec3(barrel_transform_.lock()->resolve()[3]), cannonball_initial_velocity_, getOwner()->getId());
            aiming_ = false;
        }
    }
}

bool CannonPlayerController::render() const
{
    if (Singleton::debug)
    {
        PUSH_CLEAR_STATE();

        Singleton::active_camera.lock()->bind();

        const auto position = glm::vec3(barrel_transform_.lock()->resolve()[3]);
        const auto trajectory = Physics::simulateCannonballTrajectory(position, cannonball_initial_velocity_);

        constexpr const GLfloat material[] = {_v4(color::BLUE)};
        glMaterialfv(GL_FRONT, GL_AMBIENT, material);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, material);

        glBegin(GL_LINE_STRIP);
        for (const auto &trajectory_position : trajectory)
        {
            glVertex3f(_v3(trajectory_position));
        }
        glEnd();

        POP_CLEAR_STATE();
    }
    return false;
}
