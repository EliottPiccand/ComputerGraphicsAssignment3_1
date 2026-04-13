#include "Assets/Texture.h"

#include <stdexcept>

#include <stb/stb_image.h>

#include "Utils/Log.h"

using namespace asset;

Texture::Texture(GLuint id) : id(id)
{
}

Texture::~Texture()
{
    if (id != 0)
    {
        glDeleteTextures(1, &id);
    }
}

std::shared_ptr<Texture> Texture::load(const std::filesystem::path &path)
{
    int width, height, channels;
    unsigned char *data = stbi_load(path.generic_string().c_str(), &width, &height, &channels, 0);

    if (data == nullptr)
    {
        LOG_ERROR("failed to load texture '{}'", path.string());
        throw std::runtime_error("failed to load texture");
    }

    GLint format;
    switch (channels) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            LOG_ERROR("failed to load texture from memory: invalid channel count {}", channels);
            throw std::runtime_error("texture loading failed");
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return std::make_shared<Texture>(id);
}

void Texture::bind(GLenum slot) const
{
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind(GLenum slot) const
{
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}
