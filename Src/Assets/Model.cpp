#include "Assets/Model.h"

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <filesystem>

#include <tinygltf/tiny_gltf_v3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Assets/AssetLoader.h"
#include "Assets/Texture.h"
#include "Utils/Log.h"

using namespace asset;

namespace
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

bool operator==(const Vertex &a, const Vertex &b)
{
    return a.position == b.position && a.normal == b.normal && a.uv == b.uv;
}

// Helper functions for node transforms
static glm::mat4 getNodeTransform(const tg3_node &node)
{
    glm::mat4 transform(1.0f);

    // Check if node has a matrix
    if (node.has_matrix)
    {
        // Load from matrix array (column-major in glm)
        const double *m = node.matrix;
        transform = glm::mat4(
            glm::vec4(m[0], m[1], m[2], m[3]),
            glm::vec4(m[4], m[5], m[6], m[7]),
            glm::vec4(m[8], m[9], m[10], m[11]),
            glm::vec4(m[12], m[13], m[14], m[15]));
    }
    else
    {
        // Build from TRS components (always present in TRS nodes)
        glm::vec3 translation(node.translation[0], node.translation[1], node.translation[2]);
        glm::quat rotation(
            static_cast<float>(node.rotation[3]), // w
            static_cast<float>(node.rotation[0]), // x
            static_cast<float>(node.rotation[1]), // y
            static_cast<float>(node.rotation[2])); // z
        glm::vec3 scale(node.scale[0], node.scale[1], node.scale[2]);

        // Compose T * R * S
        transform = glm::translate(glm::mat4(1.0f), translation);
        transform *= glm::mat4_cast(rotation);
        transform *= glm::scale(glm::mat4(1.0f), scale);
    }

    return transform;
}

// Filesystem context for tinygltf callbacks
struct FSContext
{
    std::filesystem::path baseDir;
};

// Filesystem callback implementations
static int32_t fs_file_exists(const char *path, uint32_t path_len, void *user_data)
{
    if (!path || path_len == 0)
        return 0;
    
    const auto *ctx = static_cast<FSContext *>(user_data);
    auto fullPath = ctx->baseDir / std::string(path, path_len);
    return std::filesystem::exists(fullPath) ? 1 : 0;
}

static int32_t fs_read_file(uint8_t **out_data, uint64_t *out_size, const char *path,
                             uint32_t path_len, void *user_data)
{
    if (!path || path_len == 0 || !out_data || !out_size)
        return 0;

    const auto *ctx = static_cast<FSContext *>(user_data);
    auto fullPath = ctx->baseDir / std::string(path, path_len);

    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open())
        return 0;

    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto *data = new uint8_t[size];
    file.read(reinterpret_cast<char *>(data), size);
    file.close();

    *out_data = data;
    *out_size = size;
    return 1;
}

static void fs_free_file(uint8_t *data, uint64_t size, void *user_data)
{
    delete[] data;
}

} // namespace

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

Model::Model(GLuint vao, GLuint vbo, std::vector<Mesh> meshes)
    : vao(vao), vbo(vbo), meshes(std::move(meshes))
{
}

Model::~Model()
{
    for (const auto &mesh : meshes)
    {
        glDeleteBuffers(1, &mesh.ibo);
    }
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}


std::shared_ptr<Model> Model::load(const std::filesystem::path &path)
{
    LOG_DEBUG("loading model '{}'", path.string());

    // Read file
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        LOG_ERROR("failed to open file '{}'", path.string());
        throw std::runtime_error("file open failed");
    }

    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char *>(data.data()), size);
    file.close();

    // Setup filesystem callbacks for tinygltf3
    FSContext fsContext{path.parent_path()};
    tg3_fs_callbacks fsCallbacks{};
    fsCallbacks.file_exists = fs_file_exists;
    fsCallbacks.read_file = fs_read_file;
    fsCallbacks.free_file = fs_free_file;
    fsCallbacks.user_data = &fsContext;

    // Setup parse options
    tg3_parse_options parseOptions{};
    parseOptions.fs = fsCallbacks;

    // Parse glTF
    tg3_error_code err;
    tg3_model model = {};
    tg3_error_stack errors = {};

    std::string baseDirStr = path.parent_path().string();
    err = tg3_parse_auto(&model, &errors, data.data(), static_cast<uint64_t>(size),
                          baseDirStr.c_str(), static_cast<uint32_t>(baseDirStr.length()),
                          &parseOptions);

    if (err != TG3_OK)
    {
        if (tg3_errors_has_error(&errors))
        {
            uint32_t errorCount = tg3_errors_count(&errors);
            LOG_ERROR("error{} occurred:", errorCount <= 1 ? "" : "s");
            for (uint32_t i = 0; i < errorCount; ++i)
            {
                const auto *entry = tg3_errors_get(&errors, i);
                if (entry)
                {
                    LOG_ERROR("- {}", entry->message);
                }
            }
        }
        LOG_ERROR("failed to load model '{}': {}", path.string(), static_cast<int>(err));
        throw std::runtime_error("model parsing failed");
    }

    tg3_model *gltfModel = &model;

    // Load textures manually from image URIs - this is where you have full control
    std::vector<std::shared_ptr<Texture>> textures;
    const auto modelDir = path.parent_path();

    for (uint32_t i = 0; i < gltfModel->images_count; ++i)
    {
        const auto &image = gltfModel->images[i];
        std::string_view uri(image.uri.data, image.uri.len);

        try
        {
            auto texturePath = modelDir / std::filesystem::path(uri);
            auto texture = AssetLoader::get<Texture>(texturePath.string());
            textures.push_back(texture);
            LOG_DEBUG("loaded texture: {}", uri);
        }
        catch (const std::exception &e)
        {
            LOG_DEBUG("failed to load texture '{}': {}", uri, e.what());
            textures.push_back(nullptr);
        }
    }

    // Extract vertices and indices from scene nodes with transforms
    std::vector<Vertex> vertices;
    std::unordered_map<Vertex, GLuint> vertexMap;
    std::unordered_map<int, std::vector<uint16_t>> materialIndices;

    // Get the default scene
    int32_t sceneIdx = gltfModel->default_scene;
    if (sceneIdx < 0 || sceneIdx >= static_cast<int32_t>(gltfModel->scenes_count))
    {
        LOG_DEBUG("no default scene, using first scene");
        sceneIdx = 0;
    }

    if (sceneIdx >= 0 && sceneIdx < static_cast<int32_t>(gltfModel->scenes_count))
    {
        const auto &scene = gltfModel->scenes[sceneIdx];
        LOG_DEBUG("processing scene with {} root nodes", scene.nodes_count);

        // Recursive function to process nodes
        std::function<void(int32_t, const glm::mat4 &)> processNode =
            [&](int32_t nodeIdx, const glm::mat4 &parentTransform)
        {
            if (nodeIdx < 0 || nodeIdx >= static_cast<int32_t>(gltfModel->nodes_count))
                return;

            const auto &node = gltfModel->nodes[nodeIdx];
            glm::mat4 nodeTransform = parentTransform * getNodeTransform(node);

            // If this node has a mesh, process it
            if (node.mesh >= 0 && node.mesh < static_cast<int32_t>(gltfModel->meshes_count))
            {
                const auto &mesh = gltfModel->meshes[node.mesh];
                LOG_DEBUG("processing node {} mesh with {} primitives, transform applied",
                          nodeIdx, mesh.primitives_count);

                for (uint32_t primIdx = 0; primIdx < mesh.primitives_count; ++primIdx)
                {
                    const auto &primitive = mesh.primitives[primIdx];

                    // Get vertex data pointers
                    glm::vec3 *positions = nullptr;
                    glm::vec3 *normals = nullptr;
                    glm::vec2 *uvs = nullptr;
                    size_t vertexCount = 0;

                    // Extract position, normal, and UV attributes
                    for (uint32_t attrIdx = 0; attrIdx < primitive.attributes_count; ++attrIdx)
                    {
                        const auto &attr = primitive.attributes[attrIdx];
                        std::string_view attrName(attr.key.data, attr.key.len);

                        int32_t accessorIdx = attr.value;
                        if (accessorIdx < 0 || accessorIdx >= static_cast<int32_t>(gltfModel->accessors_count))
                            continue;

                        const auto &accessor = gltfModel->accessors[accessorIdx];

                        if (accessor.buffer_view < 0 || accessor.buffer_view >= static_cast<int32_t>(gltfModel->buffer_views_count))
                            continue;

                        const auto &bufferView = gltfModel->buffer_views[accessor.buffer_view];

                        if (bufferView.buffer < 0 || bufferView.buffer >= static_cast<int32_t>(gltfModel->buffers_count))
                            continue;

                        const auto &buffer = gltfModel->buffers[bufferView.buffer];
                        const uint8_t *baseData = buffer.data.data + bufferView.byte_offset + accessor.byte_offset;

                        if (attrName == "POSITION")
                        {
                            positions = reinterpret_cast<glm::vec3 *>(const_cast<uint8_t *>(baseData));
                            vertexCount = accessor.count;
                        }
                        else if (attrName == "NORMAL")
                        {
                            normals = reinterpret_cast<glm::vec3 *>(const_cast<uint8_t *>(baseData));
                        }
                        else if (attrName == "TEXCOORD_0")
                        {
                            uvs = reinterpret_cast<glm::vec2 *>(const_cast<uint8_t *>(baseData));
                        }
                    }

                    // Process indices and build vertex list
                    if (primitive.indices >= 0 && vertexCount > 0)
                    {
                        const auto &indexAccessor = gltfModel->accessors[primitive.indices];
                        if (indexAccessor.buffer_view < 0 || indexAccessor.buffer_view >= static_cast<int32_t>(gltfModel->buffer_views_count))
                            continue;

                        const auto &indexBufferView = gltfModel->buffer_views[indexAccessor.buffer_view];
                        if (indexBufferView.buffer < 0 || indexBufferView.buffer >= static_cast<int32_t>(gltfModel->buffers_count))
                            continue;

                        const auto &indexBuffer = gltfModel->buffers[indexBufferView.buffer];
                        const uint8_t *indexData = indexBuffer.data.data + indexBufferView.byte_offset + indexAccessor.byte_offset;

                        std::vector<uint16_t> &meshIndices = materialIndices[primitive.material];
                        const size_t indexCount = indexAccessor.count;

                        // Handle both 16 and 32 bit indices
                        if (indexAccessor.component_type == 5125) // GL_UNSIGNED_INT
                        {
                            const uint32_t *indices32 = reinterpret_cast<const uint32_t *>(indexData);
                            for (size_t i = 0; i < indexCount; ++i)
                            {
                                uint32_t originalIndex = indices32[i];
                                if (originalIndex >= vertexCount)
                                    continue;

                                Vertex vertex{};
                                if (positions)
                                {
                                    // Apply node transform to position
                                    glm::vec4 transformedPos = nodeTransform * glm::vec4(positions[originalIndex], 1.0f);
                                    vertex.position = glm::vec3(transformedPos);
                                }
                                if (normals)
                                {
                                    // Apply only rotation/scale to normal (no translation)
                                    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(nodeTransform)));
                                    vertex.normal = glm::normalize(normalMatrix * normals[originalIndex]);
                                }
                                if (uvs)
                                    vertex.uv = uvs[originalIndex];

                                // Deduplication
                                if (vertexMap.find(vertex) == vertexMap.end())
                                {
                                    vertexMap[vertex] = static_cast<GLuint>(vertices.size());
                                    vertices.push_back(vertex);
                                }

                                meshIndices.push_back(static_cast<uint16_t>(vertexMap[vertex]));
                            }
                        }
                        else // GL_UNSIGNED_SHORT (default)
                        {
                            const uint16_t *indices16 = reinterpret_cast<const uint16_t *>(indexData);
                            for (size_t i = 0; i < indexCount; ++i)
                            {
                                uint16_t originalIndex = indices16[i];
                                if (originalIndex >= vertexCount)
                                    continue;

                                Vertex vertex{};
                                if (positions)
                                {
                                    // Apply node transform to position
                                    glm::vec4 transformedPos = nodeTransform * glm::vec4(positions[originalIndex], 1.0f);
                                    vertex.position = glm::vec3(transformedPos);
                                }
                                if (normals)
                                {
                                    // Apply only rotation/scale to normal (no translation)
                                    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(nodeTransform)));
                                    vertex.normal = glm::normalize(normalMatrix * normals[originalIndex]);
                                }
                                if (uvs)
                                    vertex.uv = uvs[originalIndex];

                                // Deduplication
                                if (vertexMap.find(vertex) == vertexMap.end())
                                {
                                    vertexMap[vertex] = static_cast<GLuint>(vertices.size());
                                    vertices.push_back(vertex);
                                }

                                meshIndices.push_back(static_cast<uint16_t>(vertexMap[vertex]));
                            }
                        }
                    }
                }
            }

            // Process children nodes
            for (uint32_t childIdx = 0; childIdx < node.children_count; ++childIdx)
            {
                processNode(node.children[childIdx], nodeTransform);
            }
        };

        // Process all root nodes in the scene
        for (uint32_t i = 0; i < scene.nodes_count; ++i)
        {
            processNode(scene.nodes[i], glm::mat4(1.0f));
        }
    }
    else
    {
        LOG_ERROR("no valid scene found in glTF model");
        throw std::runtime_error("no scene in model");
    }

    LOG_DEBUG("extracted {} vertices", vertices.size());
    LOG_DEBUG("material indices found: {}", materialIndices.size());
    for (const auto &[matIdx, indices] : materialIndices)
    {
        LOG_DEBUG("  material {}: {} indices", matIdx, indices.size());
    }

    // Validate we have geometry
    if (vertices.empty() || materialIndices.empty())
    {
        LOG_ERROR("model has no vertices or materials!");
        throw std::runtime_error("no geometry extracted from model");
    }

    // Create VAO and VBO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Setup vertex attributes (legacy OpenGL) - AFTER binding VBO
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, position)));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, normal)));

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, uv)));

    // Create meshes with materials
    std::vector<Model::Mesh> meshes;
    for (const auto &[matIndex, indices] : materialIndices)
    {
        GLuint ibo;
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

        // Load material
        Material material;
        if (matIndex >= 0 && matIndex < static_cast<int32_t>(gltfModel->materials_count))
        {
            const auto &gltfMat = gltfModel->materials[matIndex];

            // Base Color Factor
            material.baseColorFactor = glm::vec4(
                static_cast<float>(gltfMat.pbr_metallic_roughness.base_color_factor[0]),
                static_cast<float>(gltfMat.pbr_metallic_roughness.base_color_factor[1]),
                static_cast<float>(gltfMat.pbr_metallic_roughness.base_color_factor[2]),
                static_cast<float>(gltfMat.pbr_metallic_roughness.base_color_factor[3]));

            // Base Color Texture
            if (gltfMat.pbr_metallic_roughness.base_color_texture.index >= 0)
            {
                int32_t texIdx = gltfMat.pbr_metallic_roughness.base_color_texture.index;
                if (texIdx >= 0 && static_cast<size_t>(texIdx) < gltfModel->textures_count)
                {
                    int32_t srcIdx = gltfModel->textures[texIdx].source;
                    if (srcIdx >= 0 && static_cast<size_t>(srcIdx) < textures.size())
                    {
                        material.baseColorTexture = textures[srcIdx];
                    }
                }
            }

            // Metallic and Roughness Factors
            material.metallicFactor = static_cast<float>(gltfMat.pbr_metallic_roughness.metallic_factor);
            material.roughnessFactor = static_cast<float>(gltfMat.pbr_metallic_roughness.roughness_factor);

            // Metallic Roughness Texture
            if (gltfMat.pbr_metallic_roughness.metallic_roughness_texture.index >= 0)
            {
                int32_t texIdx = gltfMat.pbr_metallic_roughness.metallic_roughness_texture.index;
                if (texIdx >= 0 && static_cast<size_t>(texIdx) < gltfModel->textures_count)
                {
                    int32_t srcIdx = gltfModel->textures[texIdx].source;
                    if (srcIdx >= 0 && static_cast<size_t>(srcIdx) < textures.size())
                    {
                        material.metallicRoughnessTexture = textures[srcIdx];
                    }
                }
            }

            // Normal Map
            if (gltfMat.normal_texture.index >= 0)
            {
                int32_t texIdx = gltfMat.normal_texture.index;
                if (texIdx >= 0 && static_cast<size_t>(texIdx) < gltfModel->textures_count)
                {
                    int32_t srcIdx = gltfModel->textures[texIdx].source;
                    if (srcIdx >= 0 && static_cast<size_t>(srcIdx) < textures.size())
                    {
                        material.normalTexture = textures[srcIdx];
                    }
                }
            }

            // Emissive Factor
            material.emissiveFactor = glm::vec3(
                static_cast<float>(gltfMat.emissive_factor[0]),
                static_cast<float>(gltfMat.emissive_factor[1]),
                static_cast<float>(gltfMat.emissive_factor[2]));

            // Emissive Texture
            if (gltfMat.emissive_texture.index >= 0)
            {
                int32_t texIdx = gltfMat.emissive_texture.index;
                if (texIdx >= 0 && static_cast<size_t>(texIdx) < gltfModel->textures_count)
                {
                    int32_t srcIdx = gltfModel->textures[texIdx].source;
                    if (srcIdx >= 0 && static_cast<size_t>(srcIdx) < textures.size())
                    {
                        material.emissiveTexture = textures[srcIdx];
                    }
                }
            }

            // Ambient Occlusion
            if (gltfMat.occlusion_texture.index >= 0)
            {
                int32_t texIdx = gltfMat.occlusion_texture.index;
                if (texIdx >= 0 && static_cast<size_t>(texIdx) < gltfModel->textures_count)
                {
                    int32_t srcIdx = gltfModel->textures[texIdx].source;
                    if (srcIdx >= 0 && static_cast<size_t>(srcIdx) < textures.size())
                    {
                        material.aoTexture = textures[srcIdx];
                    }
                }
            }

            LOG_DEBUG("loaded material with {} textures",
                (material.baseColorTexture ? 1 : 0) +
                (material.metallicRoughnessTexture ? 1 : 0) +
                (material.normalTexture ? 1 : 0) +
                (material.emissiveTexture ? 1 : 0) +
                (material.aoTexture ? 1 : 0));
        }

        meshes.push_back({ibo, indices.size(), material});
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Validate created meshes
    if (meshes.empty())
    {
        LOG_ERROR("no meshes were created!");
        throw std::runtime_error("no meshes created from geometry data");
    }

    for (size_t i = 0; i < meshes.size(); ++i)
    {
        LOG_DEBUG("mesh {}: {} indices", i, meshes[i].indexCount);
    }

    // Cleanup model
    tg3_model_free(gltfModel);

    LOG_DEBUG("loaded model with {} meshes", meshes.size());

    return std::make_shared<Model>(vao, vbo, meshes);
}


void Model::draw() const
{
    if (meshes.empty())
    {
        LOG_WARNING("draw() called on model with no meshes");
        return;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    for (size_t meshIdx = 0; meshIdx < meshes.size(); ++meshIdx)
    {
        const auto &mesh = meshes[meshIdx];
        const auto &mat = mesh.material;

        // Set material properties
        GLfloat ambientColor[] = {mat.baseColorFactor.x, mat.baseColorFactor.y, mat.baseColorFactor.z, mat.baseColorFactor.w};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ambientColor);

        // Bind base color texture (slot 0)
        if (mat.baseColorTexture)
        {
            mat.baseColorTexture->bind(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
        }
        else
        {
            glColor4f(mat.baseColorFactor.x, mat.baseColorFactor.y, mat.baseColorFactor.z, mat.baseColorFactor.w);
            glDisable(GL_TEXTURE_2D);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_SHORT, nullptr);

        // Unbind texture
        if (mat.baseColorTexture)
        {
            mat.baseColorTexture->unbind(GL_TEXTURE0);
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
