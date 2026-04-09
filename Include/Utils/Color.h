#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

using Color = glm::vec4;
#define rgba(r, g, b, a) Color { r##.0f / 255.0f, g##.0f / 255.0f, b##.0f / 255.0f, a##.0f / 255.0f }
#define rgb(r, g, b) rgba(r, g, b, 255)

Color fromArray(const float array[3]);
const GLfloat *toArray(const Color &color);
