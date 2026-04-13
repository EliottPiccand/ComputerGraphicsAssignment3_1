#pragma once

#include <Lib/OpenGL.h>
#include <Lib/glm.h>

using Color = glm::vec4;

#define rgba(r, g, b, a) Color{r##.0f / 255.0f, g##.0f / 255.0f, b##.0f / 255.0f, a##.0f / 255.0f}
#define rgb(r, g, b) rgba(r, g, b, 255)

namespace color
{

constexpr const Color WHITE = rgb(255, 255, 255);
constexpr const Color TRANSPARENT = rgba(0, 0, 0, 0);

} // namespace color
