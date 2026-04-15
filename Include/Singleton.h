#pragma once

#include <memory>

#include "Components/Camera3D.h"

enum class RenderingStyle
{
    OpaquePolygon,
    Wireframe,
};

enum class View
{
    FreeCamera,
    Top,
    Cannon,
    CannonBall,
};

struct Singleton
{
    static inline bool game_loaded = false;
    static inline bool debug = false;

    /// active_camera = debug_camera if active or main camera 
    static inline std::weak_ptr<component::Camera3D> main_camera;
    static inline std::weak_ptr<component::Camera3D> active_camera;
    
    static inline RenderingStyle rendering_style = RenderingStyle::OpaquePolygon;
    static inline View view;
};
