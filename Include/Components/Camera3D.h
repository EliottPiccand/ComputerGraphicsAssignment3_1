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
    };

    struct Orthographic
    {
        double scale;
        double near;
        double far;
    };
    using Data = std::variant<Perspective, Orthographic>;

    Camera3D(Perspective perspective, const glm::vec3 &look_at);
    Camera3D(Orthographic orthographic, const glm::vec3 &look_at);

    void initialize() override;

    static void onViewportResize(uint32_t width, uint32_t height);
    void bind() const;

    glm::vec3 screenToWorld(const glm::vec2 &screen_position) const;
    glm::vec3 forward() const;

  private:
    static inline bool static_initialized_ = false;

    static inline float viewport_width;
    static inline float viewport_height;
    static inline double aspect_ratio_;

    Data data_;
    glm::vec3 look_at_;

    std::weak_ptr<component::Transform> transform_;
    friend FreeViewControls;

    Camera3D(Data data, const glm::vec3 &look_at);

    [[nodiscard]] glm::vec3 getPosition() const;
};

} // namespace component
