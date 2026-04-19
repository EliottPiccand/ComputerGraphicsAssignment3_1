#include "Components/ShipPlayerController.h"

#include <Lib/glfw.h>

#include "GameObject.h" // IWYU pragma: keep
#include "Input.h"
#include "Singleton.h"
#include "Utils/Constants.h"
#include "Utils/Math.h"
#include "Utils/View.h"

using namespace component;

ShipPlayerController::ShipPlayerController() : speed_state_(SpeedState::Stopped)
{
    Input::bindKey(Input::Action::SpeedUp, GLFW_KEY_W);
    Input::bindKey(Input::Action::TurnLeft, GLFW_KEY_A);
    Input::bindKey(Input::Action::SpeedDown, GLFW_KEY_S);
    Input::bindKey(Input::Action::TurnRight, GLFW_KEY_D);
}

void ShipPlayerController::initialize()
{
    GET_COMPONENT(Transform, transform_, ShipPlayerController);
    GET_COMPONENT(RigidBody, rigid_body_, ShipPlayerController);
}

void ShipPlayerController::update(float delta_time)
{
    (void)delta_time;

    const auto transform = transform_.lock();
    const auto rigid_body = rigid_body_.lock();

    if (Singleton::view == View::Top)
    {
        // Update speed state
        if (Input::getState(Input::Action::SpeedUp) == Input::State::JustReleased)
        {
            speed_state_ = speed_state_ == SpeedState::Backward ? SpeedState::Stopped : SpeedState::Forward;
        }
        if (Input::getState(Input::Action::SpeedDown) == Input::State::JustReleased)
        {
            speed_state_ = speed_state_ == SpeedState::Forward ? SpeedState::Stopped : SpeedState::Backward;
        }

        if (Input::getState(Input::Action::TurnLeft) == Input::State::JustReleased)
        {
            transform->rotate(SHIP_TURN_STEP, UP);
            rigid_body->setOrientation(transform->getRotation());
        }
        if (Input::getState(Input::Action::TurnRight) == Input::State::JustReleased)
        {
            transform->rotate(-SHIP_TURN_STEP, UP);
            rigid_body->setOrientation(transform->getRotation());
        }
    }

    // Update rigid body
    const auto &current_velocity = rigid_body->getVelocity();
    auto forward = getForwardVector(transform->getRotation());
    forward -= UP * glm::dot(forward, UP);
    if (glm::length(forward) <= EPSILON)
    {
        forward = NORTH;
    }
    else
    {
        forward = glm::normalize(forward);
    }

    switch (speed_state_)
    {
    case SpeedState::Forward:
        rigid_body->setVelocity(UP * glm::dot(current_velocity, UP) + forward * SHIP_SPEED);
        break;
    case SpeedState::Stopped:
        rigid_body->setVelocity(UP * glm::dot(current_velocity, UP));
        break;
    case SpeedState::Backward:
        rigid_body->setVelocity(UP * glm::dot(current_velocity, UP) - forward * SHIP_SPEED);
        break;
    }
}
