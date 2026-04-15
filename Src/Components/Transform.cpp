#include "Components/Transform.h"

#include <Lib/OpenGL.h>
#include <Lib/glm.h>

#include "GameObject.h"
#include "Utils/Constants.h"
#include "Utils/Profiling.h"

using namespace component;

Transform::Transform(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale)
    : position_(position), rotation_(rotation), scale_(scale)
{
}
Transform::Transform(const glm::vec3 &position, const glm::vec3 &rotation)
    : Transform(position, rotation, {1.0f, 1.0f, 1.0f})
{
}
Transform::Transform(const glm::vec3 &position) : Transform(position, {0.0f, 0.0f, 0.0f})
{
}
Transform::Transform() : Transform({0.0f, 0.0f, 0.0f})
{
}

glm::mat4 Transform::resolve() const
{
    ProfileScope;

    glm::mat4 transform;
    const auto ownerParentOpt = owner_.lock()->getParent();

    if (ownerParentOpt.has_value())
    {
        const auto previousTransformOpt = ownerParentOpt.value()->findFirstComponentInParents<Transform>();
        if (previousTransformOpt.has_value())
        {
            transform = previousTransformOpt.value()->resolve();
        }
        else
        {
            transform = glm::mat4(1.0f);
        }
    }
    else
    {
        transform = glm::mat4(1.0f);
    }

    transform = glm::translate(transform, position_);
    transform = transform * glm::mat4_cast(rotation_);
    transform = glm::scale(transform, scale_);

    return transform;
}

void Transform::translate(const glm::vec3 &by)
{
    position_ += by;
}

void Transform::rotate(const float angle, const glm::vec3 &axis)
{
    rotation_ = glm::angleAxis(angle, axis) * rotation_;
}

void Transform::setPosition(const glm::vec3 &position)
{
    position_ = position;
}

void Transform::pointToward(const glm::vec3 &direction)
{
    rotation_ = glm::quatLookAt(direction, UP);
}

bool Transform::render() const
{
    ProfileScope;
    ProfileScopeGPU("Transform::render");

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glm::mat4 rotation_matrix = glm::mat4_cast(rotation_);

    glTranslatef(_v3(position_));
    glMultMatrixf(glm::value_ptr(rotation_matrix));
    glScalef(_v3(scale_));

    return true;
}
