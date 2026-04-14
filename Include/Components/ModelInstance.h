#pragma once

#include <memory>

#include "Assets/Model.h"
#include "Components/Component.h"

namespace component
{

class ModelInstance : public Component
{
  public:
    ModelInstance(std::shared_ptr<asset::Model> model, asset::Model::TextureOverride texture_override = {});

    bool render() const override;

  private:
    std::shared_ptr<asset::Model> model_;
    asset::Model::TextureOverride texture_override_;
};

} // namespace component
