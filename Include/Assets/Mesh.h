#pragma once

#include <filesystem>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
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

        size_t h4 = std::hash<float>{}(v.normal.x);
        size_t h5 = std::hash<float>{}(v.normal.y);
        size_t h6 = std::hash<float>{}(v.normal.z);

        size_t h7 = std::hash<float>{}(v.uv.x);
        size_t h8 = std::hash<float>{}(v.uv.y);

        size_t seed = h1;
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h5 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h6 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h7 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h8 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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

    GLuint texture;
    glm::vec3 diffuseColor;

  public:
    Mesh(GLuint vao, GLuint vbo, GLuint ibo, size_t indexCount, GLuint texture, glm::vec3 diffuseColor);
    ~Mesh();

    static std::shared_ptr<Mesh> load(const std::filesystem::path &path);

    void draw() const;
};
