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
    transform = glm::rotate(transform, rotation_.x, X);
    transform = glm::rotate(transform, rotation_.y, Y);
    transform = glm::rotate(transform, rotation_.z, Z);
    transform = glm::scale(transform, scale_);

    return transform;
}

void Transform::translate(const glm::vec3 &by) {
    position_ += by;
}

bool Transform::render() const
{
    ProfileScope;
    ProfileScopeGPU("Transform::render");

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(_v3(position_));
    glRotatef(glm::degrees(rotation_.x), _v3(X));
    glRotatef(glm::degrees(rotation_.y), _v3(Y));
    glRotatef(glm::degrees(rotation_.z), _v3(Z));
    glScalef(_v3(scale_));

    return true;
}
