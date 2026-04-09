#pragma once

#include <cstddef>

#include "Utils/Time.h"

class Clock
{
  private:
    Instant lastFrame;

    Duration frameTimeSum;
    size_t frameCount = 0;

  public:
    Clock();

    float tick();
    [[nodiscard]] const float getFps();
};
