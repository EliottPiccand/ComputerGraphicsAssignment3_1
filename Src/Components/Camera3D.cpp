#include "Components/Camera3D.h"

#include <Lib/OpenGL.h>

#include "Events/EventQueue.h"
#include "Events/WindowResized.h"
#include "GameObject.h" // IWYU pragma: keep
#include "Utils/Constants.h"
#include "Utils/Profiling.h"

using namespace component;

Camera3D::Camera3D(Perspective perspective) : data_(perspective)
{
    if (!static_initialized_)
    {
        EventQueue::registerCallback<event::WindowResized>(
            [](const event::WindowResized &event) { onViewportResize(event.width, event.height); });
        static_initialized_ = true;
    }
}

void Camera3D::initialize()
{
    GET_COMPONENT(Transform, transform_, Camera3D);
}

void Camera3D::onViewportResize(uint32_t width, uint32_t height)
{
    aspect_ratio_ = static_cast<double>(width) / static_cast<double>(height);
}

glm::vec3 Camera3D::getPosition() const
{
    return glm::vec3(transform_.lock()->resolve()[3]);
}

void Camera3D::bind() const
{
    ProfileScope;
    ProfileScopeGPU("Camera3D::bind");

    const auto eye = getPosition();

    if (std::holds_alternative<Perspective>(data_))
    {
        const auto &perspective = std::get<Perspective>(data_);

        // Projection Matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(perspective.fov, aspect_ratio_, perspective.near, perspective.far);

        // View Matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(_dv3(eye), _dv3(perspective.look_at), _dv3(UP));
    }
}
