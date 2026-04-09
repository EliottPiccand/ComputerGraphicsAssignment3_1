#include "Utils/Color.h"

Color fromArray(const float array[3]) {
    return {array[0], array[1], array[2], 1.0f};
}

const float *toArray(const Color &color) {
    return reinterpret_cast<const float *>(&color);
}
