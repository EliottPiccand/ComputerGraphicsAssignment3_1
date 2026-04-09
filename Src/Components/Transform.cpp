#include "Components/Transform.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "GameObject.h"
#include "Utils/Constants.h"
#include "Utils/Profiling.h"


using namespace component;

Transform::Transform(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale)
    : position(position), rotation(rotation), scale(scale)
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
    const auto ownerParentOpt = owner.lock()->getParent();

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

    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, rotation.x, X);
    transform = glm::rotate(transform, rotation.y, Y);
    transform = glm::rotate(transform, rotation.z, Z);
    transform = glm::scale(transform, scale);

    return transform;
}

void Transform::translate(const glm::vec3 &by) {
    position += by;
}

bool Transform::render() const
{
    ProfileScope;
    ProfileScopeGPU("Transform::render");

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glRotatef(glm::degrees(rotation.x), X.x, X.y, X.z);
    glRotatef(glm::degrees(rotation.y), Y.x, Y.y, Y.z);
    glRotatef(glm::degrees(rotation.z), Z.x, Z.y, Z.z);
    glScalef(scale.x, scale.y, scale.z);

    return true;
}
