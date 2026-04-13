#pragma once

#include <memory>

#include "Assets/Model.h"
#include "Components/Component.h"

namespace component
{

class ModelInstance : public Component
{
  public:
    ModelInstance(std::shared_ptr<asset::Model> model);

    bool render() const override;

  private:
    std::shared_ptr<asset::Model> model_;
};

} // namespace component
