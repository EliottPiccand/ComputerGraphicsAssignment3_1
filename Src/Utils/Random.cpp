#include "Utils/Random.h"

void Random::initialize()
{
    generator = std::mt19937(randomDevice());
}

float Random::random(float min, float max)
{
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}

int Random::randint(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}
