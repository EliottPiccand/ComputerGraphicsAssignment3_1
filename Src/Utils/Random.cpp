#include "Utils/Random.h"

void Random::initialize()
{
    generator_ = std::mt19937(random_device_());
}

float Random::random(float min, float max)
{
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator_);
}

int Random::randint(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator_);
}
