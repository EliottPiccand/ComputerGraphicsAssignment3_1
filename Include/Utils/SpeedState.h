#pragma once

#include <Lib/glm.h>

enum class SpeedState
{
    Forward,
    Stopped,
    Backward,
};

constexpr const float SHIP_SPEED = 6.0f;                    // m/s
constexpr const float SHIP_TURN_STEP = glm::radians(15.0f); // rad
