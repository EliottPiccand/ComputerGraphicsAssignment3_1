#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>

#include <Lib/OpenGL.h>
#include <Lib/glm.h>

#include "Assets/Texture.h"
#include "Utils/Color.h"

namespace asset
{

class Model
{
  private:
    struct Mesh;

  public:
    Model(GLuint vertex_array, GLuint vertex_buffer, std::vector<Mesh> meshes);
    ~Model();

    [[nodiscard]] static std::shared_ptr<Model> load(const std::filesystem::path &path);

    void draw() const;

  private:
    struct Material
    {
        std::shared_ptr<Texture> base_color_texture;
        std::shared_ptr<Texture> metallic_roughness_texture;
        std::shared_ptr<Texture> normal_texture;
        std::shared_ptr<Texture> emissive_texture;
        std::shared_ptr<Texture> ambient_occlusion_texture;

        Color base_color = color::WHITE;
        float metallic_factor = 1.0f;
        float roughness = 1.0f;
        Color emissive_factor = color::TRANSPARENT;
    };

    struct Mesh
    {
        GLuint index_buffer;
        size_t index_count;
        Material material;
    };

    const GLuint vertex_array_;
    const GLuint vertex_buffer_;

    const std::vector<Mesh> meshes_;
};

} // namespace asset
