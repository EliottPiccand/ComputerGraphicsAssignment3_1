#include "Components/FreeViewControls.h"

#include <variant>

#include <Lib/glm.h>

#include "Components/Camera3D.h"
#include "GameObject.h" // IWYU pragma: keep
#include "Input.h"
#include "Singleton.h"
#include "Utils/Constants.h"
#include "Utils/Math.h"

using namespace component;

FreeViewControls::FreeViewControls()
{
    Input::bindKey(Input::Action::FreeViewForward, GLFW_KEY_W);
    Input::bindKey(Input::Action::FreeViewLeft, GLFW_KEY_A);
    Input::bindKey(Input::Action::FreeViewBackward, GLFW_KEY_S);
    Input::bindKey(Input::Action::FreeViewRight, GLFW_KEY_D);
    Input::bindKey(Input::Action::FreeViewUp, GLFW_KEY_SPACE);
    Input::bindKey(Input::Action::FreeViewDown, GLFW_KEY_LEFT_SHIFT);
}

void FreeViewControls::update(float delta_time)
{
    constexpr const float SPEED = 6.0f; // m/s
    constexpr const float VERTICAL_SENSITIVITY = 0.3f;
    constexpr const float HORIZONTAL_SENSITIVITY = 0.3f;

    if (!active)
    {
        return;
    }

    const auto &camera = Singleton::active_camera.lock();
    if (std::holds_alternative<Camera3D::Perspective>(camera->data_))
    {
        const auto cameraPosition = camera->getPosition();
        auto camera_direction = glm::normalize(camera->look_at_ - cameraPosition);
        auto forward =
            glm::normalize(glm::dot(camera_direction, NORTH) * NORTH + glm::dot(camera_direction, EAST) * EAST);
        auto right = glm::cross(forward, UP);

        // rotation
        auto mouse_delta = Input::getMouseDelta();
        if (glm::length(mouse_delta) > EPSILON)
        {
            camera_direction = glm::rotate(camera_direction, mouse_delta.y * delta_time * VERTICAL_SENSITIVITY, right);
            camera_direction = glm::rotate(camera_direction, mouse_delta.x * delta_time * HORIZONTAL_SENSITIVITY, UP);

            camera->look_at_ = cameraPosition + camera_direction;

            forward = glm::normalize(glm::dot(camera_direction, NORTH) * NORTH + glm::dot(camera_direction, EAST) * EAST);
            right = glm::cross(forward, UP);
        }

        // motion
        glm::vec3 motion{};

        if (Input::isPressed(Input::Action::FreeViewForward))
        {
            motion += forward;
        }
        if (Input::isPressed(Input::Action::FreeViewLeft))
        {
            motion -= right;
        }
        if (Input::isPressed(Input::Action::FreeViewBackward))
        {
            motion -= forward;
        }
        if (Input::isPressed(Input::Action::FreeViewRight))
        {
            motion += right;
        }

        if (glm::length(motion) > EPSILON)
        {
            motion = glm::normalize(motion);
        }

        if (Input::isPressed(Input::Action::FreeViewUp))
        {
            motion += UP;
        }
        if (Input::isPressed(Input::Action::FreeViewDown))
        {
            motion -= UP;
        }

        if (glm::length(motion) > EPSILON)
        {
            motion *= SPEED * delta_time;

            camera->look_at_ += motion;
            camera->transform_.lock()->translate(motion);
        }
    }
}
