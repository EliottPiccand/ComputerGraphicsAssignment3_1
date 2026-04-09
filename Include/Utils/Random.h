#pragma once

#include <random>
#include <vector>

#include "Utils/Ranges.h"

class Random
{
  private:
    static inline std::random_device randomDevice;
    static inline std::mt19937 generator;

  public:
    static void initialize();
    [[nodiscard]] static float random(float min, float max);
    [[nodiscard]] static int randint(int min, int max);

    template <Range R> [[nodiscard]] static size_t index(const R &range);

    template <typename T, RangeOf<T> R> [[nodiscard]] static const T &range(const R &range);
    template <typename T> [[nodiscard]] static T pop(std::vector<T> &range);
};

template <Range R> size_t Random::index(const R &range)
{
    std::uniform_int_distribution<size_t> distribution(0, range.size() - 1);
    return distribution(generator);
}

template <typename T, RangeOf<T> R> const T &Random::range(const R &range)
{
    return range[Random::index(range)];
}

template <typename T> T Random::pop(std::vector<T> &range)
{
    size_t index = Random::index(range);
    T value = range[index];

    range.erase(range.begin() + index);

    return value;
}
