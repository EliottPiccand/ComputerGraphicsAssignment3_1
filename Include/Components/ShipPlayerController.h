#pragma once

#include "Components/ShipController.h"

namespace component
{

class ShipPlayerController : public ShipController
{
  public:
    ShipPlayerController();

  protected:
    void updateStates() override;

};

} // namespace component
