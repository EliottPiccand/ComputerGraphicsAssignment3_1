#include "Components/ShipController.h"

#include "GameObject.h" // IWYU pragma: keep
#include "Utils/Math.h"

using namespace component;

ShipController::ShipController()
    : speed_state_(SpeedState::Stopped), turn_state_(TurnState::None), turn_speed_(SHIP_TURN_STEP)
{
}

void ShipController::initialize()
{
    GET_COMPONENT(Transform, transform_, ShipController);
    GET_COMPONENT(RigidBody, rigid_body_, ShipController);
}

void ShipController::update(float delta_time)
{
    (void)delta_time;

    updateStates();

    const auto transform = transform_.lock();
    const auto rigid_body = rigid_body_.lock();

    if (turn_state_ == TurnState::Left)
    {
        transform->rotate(turn_speed_, UP);
        rigid_body->setOrientation(transform->getRotation());
    }
    if (turn_state_ == TurnState::Right)
    {
        transform->rotate(-turn_speed_, UP);
        rigid_body->setOrientation(transform->getRotation());
    }

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

void ShipController::updateStates()
{
}
