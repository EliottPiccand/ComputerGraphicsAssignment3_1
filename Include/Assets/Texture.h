#pragma once

#include <filesystem>
#include <memory>

#include <Lib/OpenGL.h>

namespace asset
{

class Texture
{
  public:
    enum class Type {
        Albedo,
        MetallicSmoothness,
        Normal,
        Emissive,
    };

    Texture(GLuint id);
    ~Texture();

    [[nodiscard]] static std::shared_ptr<Texture> load(const std::filesystem::path &path);

    void bind(GLenum slot) const;
    void unbind(GLenum slot) const;

  private:
    const GLuint id_;
};

} // namespace asset
