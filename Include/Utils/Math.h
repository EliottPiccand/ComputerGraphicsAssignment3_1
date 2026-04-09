#pragma once

template <typename T> [[nodiscard]] T lerp(const T &start, const T &end, float t)
{
    return start + t * (end - start);
}
