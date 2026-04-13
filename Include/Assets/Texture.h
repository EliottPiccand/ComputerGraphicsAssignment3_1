#pragma once

#include <filesystem>
#include <memory>

#include <GL/glew.h>

namespace asset
{

class Texture
{
  private:
    GLuint id;

  public:
    Texture(GLuint id);
    ~Texture();

    static std::shared_ptr<Texture> load(const std::filesystem::path &path);

    void bind(GLenum slot) const;
    void unbind(GLenum slot) const;
};

} // namespace asset
