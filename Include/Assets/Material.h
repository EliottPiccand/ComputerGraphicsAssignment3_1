#pragma once

#include "Utils/Color.h"
#include <filesystem>
#include <memory>

#include <GL/glew.h>
#include <tinyobjloader/tiny_obj_loader.h>

class Material
{
  private:
    GLuint diffuseTexture;

    Color ambient;
    Color diffuse;
    Color specular;
    float shyniness;

  public:
    Material(GLuint diffuseTexture, Color ambient, Color diffuse, Color specular, float shyniness);
    ~Material();

    static std::shared_ptr<Material> load(const std::filesystem::path &, const tinyobj::material_t &materialInfo);

    void bind() const;
    void unbind() const;
};
