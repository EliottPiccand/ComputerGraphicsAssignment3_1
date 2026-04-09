#pragma once

#include <memory>

#include "Assets/Mesh.h"
#include "Components/Component.h"

namespace component
{

class MeshInstance : public Component
{
  private:
    std::shared_ptr<Mesh> mesh;

  public:
    MeshInstance(std::shared_ptr<Mesh> mesh);

    bool render() const override;
};

} // namespace component
