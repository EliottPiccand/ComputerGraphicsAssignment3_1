#include "Utils/MeshPrimitives.h"

Mesh generateQuadPlane(float side_length, size_t quads_per_side)
{
    const size_t vertex_count = (quads_per_side + 1) * (quads_per_side + 1);
    const size_t degenerated_triangles_count = 2 * (quads_per_side - 1);

    std::vector<Vertex> vertices;
    vertices.reserve(vertex_count);
    std::vector<uint16_t> indices;
    indices.reserve(vertex_count + degenerated_triangles_count);

    float half_size = 0.5f * side_length;
    float quad_size = side_length / static_cast<float>(quads_per_side);

    // Generate vertices
    for (size_t y = 0; y <= quads_per_side; ++y)
    {
        for (size_t x = 0; x <= quads_per_side; ++x)
        {
            vertices.push_back({
                .position =
                    {
                        static_cast<float>(x) * quad_size - half_size,
                        static_cast<float>(y) * quad_size - half_size,
                        0.0f,
                    },
            });
        }
    }

    for (size_t y = 0; y < quads_per_side; ++y)
    {
        if (y > 0)
        {
            // Degenerate: repeat first vertex of new row
            indices.push_back(static_cast<uint16_t>(y * (quads_per_side + 1)));
        }
        for (size_t x = 0; x <= quads_per_side; ++x)
        {
            indices.push_back(static_cast<uint16_t>((y + 1) * (quads_per_side + 1) + x));
            indices.push_back(static_cast<uint16_t>(y * (quads_per_side + 1) + x));
        }
        if (y < quads_per_side - 1)
        {
            // Degenerate: repeat last vertex of current row
            indices.push_back(static_cast<uint16_t>(((y + 1) * (quads_per_side + 1) + quads_per_side)));
        }
    }

    return {vertices, indices};
}
