#pragma once

#include <memory>

#include "Assets/Model.h"
#include "Components/Component.h"

namespace component
{

class ModelInstance : public Component
{
  private:
    std::shared_ptr<asset::Model> model;

  public:
    ModelInstance(std::shared_ptr<asset::Model> model);

    bool render() const override;
};

} // namespace component
