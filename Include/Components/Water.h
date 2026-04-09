#pragma once

#include "Components/Component.h"

namespace component
{

class Water : public Component
{
  public:
    bool render() const override;
};

} // namespace component
