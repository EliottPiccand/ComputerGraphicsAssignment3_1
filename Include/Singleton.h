#pragma once

#include <memory>

#include "Components/Camera3D.h"

struct Singleton
{
    static inline bool gameLoaded = false;
    static inline std::weak_ptr<component::Camera3D> activeCamera;
};
