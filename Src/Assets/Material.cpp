#include "Assets/Material.h"

#include <stb/stb_image.h>

#include "Utils/Color.h"
#include "Utils/Log.h"

Material::Material(GLuint diffuseTexture, Color ambient, Color diffuse, Color specular, float shyniness)
    : diffuseTexture(diffuseTexture), ambient(ambient), diffuse(diffuse), specular(specular), shyniness(shyniness)
{
}

Material::~Material()
{
    if (diffuseTexture != 0)
    {
        glDeleteTextures(1, &diffuseTexture);
    }
}

std::shared_ptr<Material> Material::load(const std::filesystem::path &, const tinyobj::material_t &materialInfo)
{
    GLuint diffuseTexture = 0;

    if (!materialInfo.diffuse_texname.empty())
    {
        std::string texturePath = materialInfo.diffuse_texname;

        int width, height, channels;
        unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &channels, 4); // Force RGBA

        if (data)
        {
            glGenTextures(1, &diffuseTexture);
            glBindTexture(GL_TEXTURE_2D, diffuseTexture);

            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // Set texture environment
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

            // Upload data
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
            LOG_DEBUG("Loaded texture: {}", texturePath);
        }
        else
        {
            LOG_ERROR("Failed to load texture: {}", texturePath);
        }
    }

    return std::make_shared<Material>(diffuseTexture, fromArray(materialInfo.ambient), fromArray(materialInfo.diffuse),
                                      fromArray(materialInfo.specular), materialInfo.shininess);
}

void Material::bind() const
{
    if (diffuseTexture != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
    }

    glMaterialfv(GL_FRONT, GL_AMBIENT, toArray(ambient));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, toArray(diffuse));
    glMaterialfv(GL_FRONT, GL_SPECULAR, toArray(specular));
    glMaterialfv(GL_FRONT, GL_SHININESS, &shyniness);
}

void Material::unbind() const
{
    if (diffuseTexture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
}
