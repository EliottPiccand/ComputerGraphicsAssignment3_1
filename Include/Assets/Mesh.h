#pragma once

#include <filesystem>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
};

bool operator==(const Vertex &a, const Vertex &b);

namespace std
{
template <> struct hash<Vertex>
{
    size_t operator()(const Vertex &v) const noexcept
    {
        size_t h1 = std::hash<float>{}(v.position.x);
        size_t h2 = std::hash<float>{}(v.position.y);
        size_t h3 = std::hash<float>{}(v.position.z);

        size_t seed = h1;
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std

class Mesh
{
  private:
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    size_t indexCount;

  public:
    Mesh(GLuint vao, GLuint vbo, GLuint ibo, size_t indexCount);
    ~Mesh();

    static std::shared_ptr<Mesh> load(const std::filesystem::path &path);

    void draw() const;
};
