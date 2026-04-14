#pragma once

#include <Lib/glm.h>

#include "Components/Component.h"

namespace component
{

class Transform : public Component
{
  public:
    Transform(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);
    Transform(const glm::vec3 &position, const glm::vec3 &rotation);
    Transform(const glm::vec3 &position);
    Transform();

    [[nodiscard]] glm::mat4 resolve() const;

    void translate(const glm::vec3 &by);
    /// axis should be normalized
    void rotate(const float angle, const glm::vec3 &axis);

    bool render() const override;

  private:
    glm::vec3 position_;
    glm::quat rotation_;
    glm::vec3 scale_;
};

} // namespace component
