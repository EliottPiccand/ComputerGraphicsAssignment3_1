#include "Assets/Mesh.h"

#include <cstdint>
#include <string>

#include <tinyobjloader/tiny_obj_loader.h>

#include "Utils/Log.h"

bool operator==(const Vertex &a, const Vertex &b)
{
    return a.position == b.position && a.normal == b.normal;
}

Mesh::Mesh(GLuint vao, GLuint vbo, GLuint ibo, size_t indexCount) : vao(vao), vbo(vbo), ibo(ibo), indexCount(indexCount)
{
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

std::shared_ptr<Mesh> Mesh::load(const std::filesystem::path &path)
{
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
            };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint16_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    LOG_DEBUG("loaded model {} with {} vertices and {} indices", path.string(), vertices.size(), indices.size());

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
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void*>(0));

    // bind normal
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void*>(sizeof(glm::vec3)));

    return std::make_shared<Mesh>(vbo, vao, ibo, indices.size());
}

void Mesh::draw() const
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_SHORT, 0);
}
