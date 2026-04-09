#include "Components/LightSource.h"

#include <stdexcept>

#include "GameObject.h" // IWYU pragma: keep

using namespace component;

LightSource::LightSource(Color ambient, Color diffuse) : ambient(ambient), diffuse(diffuse)
{
    if (availableLights.empty())
    {
        throw std::runtime_error("no more light source are available");
    }

    lightId = *availableLights.begin();
    availableLights.erase(availableLights.begin());

    glEnable(lightId);
}

LightSource::~LightSource()
{
    glDisable(lightId);
    availableLights.insert(lightId);
}

bool LightSource::render() const
{
    constexpr const GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};

    glLightfv(lightId, GL_POSITION, lightPosition);

    glLightfv(lightId, GL_AMBIENT, reinterpret_cast<const GLfloat*>(&ambient));
    glLightfv(lightId, GL_DIFFUSE, reinterpret_cast<const GLfloat*>(&diffuse));

    // other settings :
    // GL_SPECULAR
    // GL_CONSTANT_ATTENUATION
    // GL_LINEAR_ATTENUATION
    // GL_QUADRATIC_ATTENUATION
    // GL_SPOT_DIRECTION
    // GL_SPOT_EXPONENT
    // GL_SPOT_CUTOFF

    return false;
}