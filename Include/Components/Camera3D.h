#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include <glm/glm.hpp>

#include "Components/Component.h"
#include "Components/Transform.h"

namespace component
{

class FreeViewControls;

class Camera3D : public Component
{
  public:
    struct Perspective
    {
        float fov;
        float near;
        float far;
        glm::vec3 lookAt;
    };

  private:
    float aspectRatio;
    std::variant<Perspective> data;

    std::weak_ptr<component::Transform> transform;
    friend FreeViewControls;

    glm::vec3 getPosition() const;

  public:
    Camera3D(Perspective perspective);

    void initialize() override;

    void onViewportResize(uint32_t width, uint32_t height);
    void bind() const;
};

} // namespace component
