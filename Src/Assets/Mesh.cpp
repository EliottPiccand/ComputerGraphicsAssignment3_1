#include "Assets/Mesh.h"

#include <cstdint>
#include <string>

#include <stb/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>

#include "Utils/Log.h"

bool operator==(const Vertex &a, const Vertex &b)
{
    return a.position == b.position && a.normal == b.normal && a.uv == b.uv;
}

Mesh::Mesh(GLuint vao, GLuint vbo, GLuint ibo, size_t indexCount, GLuint texture, glm::vec3 diffuseColor)
    : vao(vao), vbo(vbo), ibo(ibo), indexCount(indexCount), texture(texture), diffuseColor(diffuseColor)
{
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
    }
}

std::shared_ptr<Mesh> Mesh::load(const std::filesystem::path &path)
{
    LOG_DEBUG("loading mesh {}", path.string());

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    auto pathAsU8String = path.u8string();
    std::string pathAsString(pathAsU8String.begin(), pathAsU8String.end());
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathAsString.c_str()))
    {
        if (warn != "")
        {
            LOG_WARNING("while loading mesh {}: {}", path.string(), warn);
        }
        LOG_ERROR("while loading mesh {}: {}", path.string(), err);
        throw std::runtime_error(err);
    }

    if (warn != "")
    {
        LOG_WARNING("while loading mesh {}: {}", path.string(), warn);
    }

    LOG_DEBUG("reading model");

    std::unordered_map<Vertex, uint16_t> uniqueVertices;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex = {
                .position =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    },
                .normal =
                    {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    },
                .uv = {},
            };

            if (index.texcoord_index >= 0)
            {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint16_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    LOG_DEBUG("loaded model with {} vertices and {} indices", vertices.size(), indices.size());

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    // bind position
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, position)));

    // bind normal
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, normal)));

    // bind uv
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, uv)));

    GLuint texture = 0;
    if (!materials.empty())
    {
        if (materials.size() > 1)
        {
            LOG_WARNING("only one material is supported, only the first one listed will be used");
        }

        const auto &material = materials[0];
        LOG_DEBUG("materials : \n- alpha_texname: {}\n- ambient_texname: {}\n- bump_texname: {}\n- diffuse_texname: "
                  "{}\n- displacement_texname: {}\n- emissive_texname: {}\n- metallic_texname: {}\n- normal_texname: "
                  "{}\n- reflection_texname: {}\n- roughness_texname: {}\n- sheen_texname: {}\n- specular_texname: "
                  "{}\n- specular_highlight_texname: {}",
                  material.alpha_texname, material.ambient_texname, material.bump_texname, material.diffuse_texname,
                  material.displacement_texname, material.emissive_texname, material.metallic_texname,
                  material.normal_texname, material.reflection_texname, material.roughness_texname,
                  material.sheen_texname, material.specular_texname, material.specular_highlight_texname);

        if (!material.diffuse_texname.empty())
        {
            std::string texturePath = materials[0].diffuse_texname;

            int width, height, channels;
            unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &channels, 4); // Force RGBA

            if (data)
            {
                LOG_DEBUG("texture loaded");

                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

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
    }

    glm::vec3 diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    if (!materials.empty())
    {
        diffuseColor = glm::vec3(materials[0].diffuse[0], materials[0].diffuse[1], materials[0].diffuse[2]);
    }

    return std::make_shared<Mesh>(vao, vbo, ibo, indices.size(), texture, diffuseColor);
}

void Mesh::draw() const
{
    if (texture != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    if (texture != 0)
    {
        glDisable(GL_TEXTURE_2D);
    }
}
