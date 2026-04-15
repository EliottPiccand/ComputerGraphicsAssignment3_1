#include "Utils/Math.h"
#include <numbers>


glm::quat twistAroundAxis(const glm::quat &q, const glm::vec3 &axis)
{
    glm::vec3 proj = axis * glm::dot(glm::vec3(q.x, q.y, q.z), axis);
    glm::quat twist(q.w, proj.x, proj.y, proj.z);
    float len = glm::length(twist);
    if (len < EPSILON)
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    return twist / len;
}

float angleAroundAxis(const glm::quat &q, const glm::vec3 &axis)
{
    glm::quat twist = twistAroundAxis(q, axis);
    return 2.0f * std::atan2(glm::dot(glm::vec3(twist.x, twist.y, twist.z), axis), twist.w) - std::numbers::pi_v<float> / 2.0f;
}
