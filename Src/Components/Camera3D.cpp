#include "Components/Camera3D.h"

#include <GL/glew.h>
#include <GL/glu.h>

#include "Events/EventQueue.h"
#include "Events/WindowResized.h"
#include "GameObject.h" // IWYU pragma: keep
#include "Utils/Constants.h"
#include "Utils/Profiling.h"

using namespace component;

Camera3D::Camera3D(Perspective perspective) : data(perspective)
{
    EventQueue::registerCallback<event::WindowResized>(
        [this](const event::WindowResized &event) { onViewportResize(event.width, event.height); });
}

void Camera3D::initialize()
{
    GET_COMPONENT(Transform, transform, Camera3D);
}

void Camera3D::onViewportResize(uint32_t width, uint32_t height)
{
    aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

glm::vec3 Camera3D::getPosition() const {
    return glm::vec3(transform.lock()->resolve()[3]);
}

void Camera3D::bind() const
{
    ProfileScope;
    ProfileScopeGPU("Camera3D::bind");

    const auto eye = getPosition();

    if (std::holds_alternative<Perspective>(data))
    {
        const auto &perspective = std::get<Perspective>(data);

        // Projection Matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(perspective.fov, aspectRatio, perspective.near, perspective.far);

        // View Matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(eye.x, eye.y, eye.z, perspective.lookAt.x, perspective.lookAt.y, perspective.lookAt.z, UP.x, UP.y,
                  UP.z);
    }
}
