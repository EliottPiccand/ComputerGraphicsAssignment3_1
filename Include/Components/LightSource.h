#pragma once

#include <unordered_set>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Components/Component.h"
#include "Utils/Color.h"

namespace component
{

class LightSource : public Component
{
  public:
    static inline constexpr const Color AMBIENT_COLOR = rgba(255, 255, 255, 1);

  private:
    static inline std::unordered_set<GLenum> availableLights = {
        GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7,
    };

    GLenum lightId;

    Color ambient;
    Color diffuse;

  public:
    LightSource(Color ambient, Color diffuse);
    ~LightSource();

    bool render() const override;
};

} // namespace component
