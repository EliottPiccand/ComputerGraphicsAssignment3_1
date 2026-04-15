#pragma once

#include <Lib/glm.h>

constexpr const float EPSILON = 1e-5f;

glm::quat twistAroundAxis(const glm::quat &q, const glm::vec3 &axis);

/// axis must be normalized
float angleAroundAxis(const glm::quat &q, const glm::vec3 &axis);
