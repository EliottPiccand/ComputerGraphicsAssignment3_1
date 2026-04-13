#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include <Lib/glm.h>

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
        double fov;
        double near;
        double far;
        glm::vec3 look_at;
    };

    Camera3D(Perspective perspective);

    void initialize() override;

    static void onViewportResize(uint32_t width, uint32_t height);
    void bind() const;

  private:
    static inline bool static_initialized_ = false;
    static inline double aspect_ratio_;

    std::variant<Perspective> data_;

    std::weak_ptr<component::Transform> transform_;
    friend FreeViewControls;

    [[nodiscard]] glm::vec3 getPosition() const;
};

} // namespace component
