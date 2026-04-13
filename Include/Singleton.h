#pragma once

#include <memory>

#include "Components/Camera3D.h"

enum class RenderingStyle
{
    OpaquePolygon,
    Wireframe,
};

struct Singleton
{
    static inline bool game_loaded = false;
    static inline std::weak_ptr<component::Camera3D> active_camera;
    static inline RenderingStyle rendering_style = RenderingStyle::OpaquePolygon;
};
