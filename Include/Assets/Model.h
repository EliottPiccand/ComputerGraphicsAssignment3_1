#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Assets/Texture.h"

namespace asset
{

class Model
{
  public:
    struct Material
    {
        // PBR Textures
        std::shared_ptr<Texture> baseColorTexture;
        std::shared_ptr<Texture> metallicRoughnessTexture;
        std::shared_ptr<Texture> normalTexture;
        std::shared_ptr<Texture> emissiveTexture;
        std::shared_ptr<Texture> aoTexture;

        // Material Factor Values
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        glm::vec3 emissiveFactor = glm::vec3(0.0f);
    };

  private:
    struct Mesh
    {
        GLuint ibo;
        size_t indexCount;
        Material material;
    };

    GLuint vao;
    GLuint vbo;

    std::vector<Mesh> meshes;

  public:
    Model(GLuint vao, GLuint vbo, std::vector<Mesh> meshes);
    ~Model();

    static std::shared_ptr<Model> load(const std::filesystem::path &path);

    void draw() const;
};

} // namespace asset
