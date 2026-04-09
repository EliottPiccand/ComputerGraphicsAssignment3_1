#include "Clock.h"

#include <chrono>

Clock::Clock() : lastFrame(now()), frameTimeSum(0)
{
}

float Clock::tick()
{
    const Duration elapsed = now() - lastFrame;

    frameCount += 1;
    frameTimeSum += elapsed;

    lastFrame = now();

    return std::chrono::duration<float>(elapsed).count();
}

const float Clock::getFps()
{
    const float fps = static_cast<float>(frameCount) / std::chrono::duration<float>(frameTimeSum).count();
    frameCount = 0;
    frameTimeSum = Duration(0);
    return fps;
}
