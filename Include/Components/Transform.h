#pragma once

#include <glm/glm.hpp>

#include "Components/Component.h"

namespace component
{

class Transform : public Component
{
  private:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

  public:
    Transform(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);
    Transform(const glm::vec3 &position, const glm::vec3 &rotation);
    Transform(const glm::vec3 &position);
    Transform();

    glm::mat4 resolve() const;

    bool render() const override;
};

} // namespace component
