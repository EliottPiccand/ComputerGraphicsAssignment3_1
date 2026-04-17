#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <Lib/glm.h>

struct Vertex
{
    glm::vec3 position;
};

using Mesh = std::pair<std::vector<Vertex>, std::vector<uint16_t>>;

// Plane facing +Z
Mesh generateQuadPlane(float side_length, size_t quads_per_side);
