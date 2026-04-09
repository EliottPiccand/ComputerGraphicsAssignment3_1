#include "Components/FreeViewControls.h"

#include <variant>

#include <glm/gtx/rotate_vector.hpp>

#include "Components/Camera3D.h"
#include "GameObject.h" // IWYU pragma: keep
#include "Input.h"
#include "Singleton.h"
#include "Utils/Constants.h"


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

void FreeViewControls::update(float deltaTime)
{
    constexpr const float SPEED = 3.0f; // m/s
    constexpr const float VERTICAL_SENSITIVITY = 0.3f;
    constexpr const float HORIZONTAL_SENSITIVITY = 0.3f;

    if (!active) {
        return;
    }

    const auto &camera = Singleton::activeCamera.lock();
    if (std::holds_alternative<Camera3D::Perspective>(camera->data)) {
        auto &perspective = std::get<Camera3D::Perspective>(camera->data);
        
        const auto cameraPosition = camera->getPosition();
        auto cameraDirection = glm::normalize(perspective.lookAt - cameraPosition);
        auto forward = glm::normalize(glm::dot(cameraDirection, NORTH) * NORTH + glm::dot(cameraDirection, EAST) * EAST);
        auto right = glm::cross(forward, UP);

        // rotation
        auto mouseDelta = Input::getMouseDelta(); 
        if (glm::length(mouseDelta) > 1e-5) {
            cameraDirection = glm::rotate(cameraDirection, mouseDelta.y * deltaTime * VERTICAL_SENSITIVITY, right);
            cameraDirection = glm::rotate(cameraDirection, mouseDelta.x * deltaTime * HORIZONTAL_SENSITIVITY, UP);
        
            perspective.lookAt = cameraPosition + cameraDirection;

            forward = glm::normalize(glm::dot(cameraDirection, NORTH) * NORTH + glm::dot(cameraDirection, EAST) * EAST);
            right = glm::cross(forward, UP);
        }

        // motion
        glm::vec3 motion{};

        if (Input::isPressed(Input::Action::FreeViewForward)) {
            motion += forward;
        }
        if (Input::isPressed(Input::Action::FreeViewLeft)) {
            motion -= right;
        }
        if (Input::isPressed(Input::Action::FreeViewBackward)) {
            motion -= forward;
        }
        if (Input::isPressed(Input::Action::FreeViewRight)) {
            motion += right;
        }

        if (glm::length(motion) > 1e-5) {
            motion = glm::normalize(motion);
        }

        if (Input::isPressed(Input::Action::FreeViewUp)) {
            motion += UP;
        }
        if (Input::isPressed(Input::Action::FreeViewDown)) {
            motion -= UP;
        }

        if (glm::length(motion) > 1e-5) {
            motion *= SPEED * deltaTime;

            perspective.lookAt += motion;
            camera->transform.lock()->translate(motion);
        }
    }
}
