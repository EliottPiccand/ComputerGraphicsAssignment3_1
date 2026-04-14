#include "Components/Camera3D.h"

#include <Lib/OpenGL.h>
#include <stdexcept>
#include <variant>

#include "Events/EventQueue.h"
#include "Events/WindowResized.h"
#include "GameObject.h" // IWYU pragma: keep
#include "Utils/Constants.h"
#include "Utils/Profiling.h"

using namespace component;

Camera3D::Camera3D(Data data, const glm::vec3 &look_at) : data_(data), look_at_(look_at)
{
    if (!static_initialized_)
    {
        EventQueue::registerCallback<event::WindowResized>(
            [](const event::WindowResized &event) { onViewportResize(event.width, event.height); });
        static_initialized_ = true;
    }
}

Camera3D::Camera3D(Perspective perspective, const glm::vec3 &look_at) : Camera3D(Data(perspective), look_at)
{
}

Camera3D::Camera3D(Orthographic orthographic, const glm::vec3 &look_at) : Camera3D(Data(orthographic), look_at)
{
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

    if (std::holds_alternative<Perspective>(data_))
    {
        const auto &perspective = std::get<Perspective>(data_);

        // Projection Matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(perspective.fov, aspect_ratio_, perspective.near, perspective.far);
    }
    else if (std::holds_alternative<Orthographic>(data_))
    {
        const auto &orthographic = std::get<Orthographic>(data_);

        double left = -orthographic.scale * aspect_ratio_;
        double right = orthographic.scale * aspect_ratio_;
        double bottom = -orthographic.scale;
        double top = orthographic.scale;

        // Projection Matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(left, right, bottom, top, orthographic.near, orthographic.far);
    }
    else
    {
        throw std::runtime_error("Camera3D type not implementd");
    }

    const auto eye = getPosition();

    // View Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(_dv3(eye), _dv3(look_at_), _dv3(UP));
}
