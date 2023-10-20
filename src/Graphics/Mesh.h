#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

struct BasicVertex
{
    glm::vec3 position{0.0f};
    glm::vec2 texture_coord{0.0f};
    glm::vec3 normal{0.0f};
};

struct BulletDebugVertex
{
    glm::vec3 position{0.0f};
    glm::vec3 colour{0.0f};
};

template <typename VertexType>
struct Mesh
{
    std::vector<VertexType> vertices;
    std::vector<GLuint> indices;
};


using BasicMesh = Mesh<BasicVertex>;
using BulletDebugMesh = Mesh<BulletDebugVertex>;

[[nodiscard]] BasicMesh generate_quad_mesh(float w, float h);
[[nodiscard]] BasicMesh generate_cube_mesh(const glm::vec3& size, bool repeat_texture);
[[nodiscard]] BasicMesh generate_terrain_mesh(int size);