#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "OpenGL/VertexArray.h"

struct HeightMap;

/// Basic vertex type for rendering
struct BasicVertex
{
    glm::vec3 position{0.0f};
    glm::vec2 texture_coord{0.0f};
    glm::vec3 normal{0.0f};

    static void link_attribs(VertexArray& vao, const BufferObject& vbo)
    {
        vao.add_attribute(vbo, sizeof(BasicVertex), 3, GL_FLOAT, offsetof(BasicVertex, position));
        vao.add_attribute(vbo, sizeof(BasicVertex), 2, GL_FLOAT,
                          offsetof(BasicVertex, texture_coord));
        vao.add_attribute(vbo, sizeof(BasicVertex), 3, GL_FLOAT, offsetof(BasicVertex, normal));
    }
};

/// Debug vertex type for rendering
struct DebugVertex
{
    glm::vec3 position{0.0f};
    glm::vec3 colour{0.0f};

    static void link_attribs(VertexArray& vao, const BufferObject& vbo)
    {
        vao.add_attribute(vbo, sizeof(DebugVertex), 3, GL_FLOAT, offsetof(DebugVertex, position));
        vao.add_attribute(vbo, sizeof(DebugVertex), 3, GL_FLOAT, offsetof(DebugVertex, colour));
    }
};

/// Mesh of a given vertex type, used to render geometry
template <typename VertexType>
class Mesh
{
  public:
    std::vector<VertexType> vertices;
    std::vector<GLuint> indices;

    void buffer();
    void update();

    void bind() const;
    void draw(GLenum draw_mode = GL_TRIANGLES) const;

  private:
    VertexArray vao_;
    BufferObject vbo_;
    BufferObject ebo_;

    GLuint indices_ = 0;

    bool buffered_ = false;
};

using BasicMesh = Mesh<BasicVertex>;
using DebugMesh = Mesh<DebugVertex>;

// =================================
// Buffers the mesh to the GPU
// =================================
template <typename VertexType>
inline void Mesh<VertexType>::buffer()
{
    if (buffered_)
    {
        return;
    }
    std::cout << "Buffering mesh\n";
    // Ensure the mesh data is reset
    vao_.reset();
    vbo_.reset();
    ebo_.reset();

    indices_ = static_cast<GLuint>(indices.size());

    // Attach EBO
    ebo_.buffer_data(indices);
    glVertexArrayElementBuffer(vao_.id, ebo_.id);

    // Attach VBO
    vbo_.buffer_data(vertices);
    VertexType::link_attribs(vao_, vbo_);

    buffered_ = true;
}

template <typename VertexType>
inline void Mesh<VertexType>::update()
{
    if (indices_ != static_cast<GLuint>(indices.size()))
    {
        std::cout << "Indides mis-match. Current: " << indices_ << " - New: " << indices.size()
                  << "\nRe-creating mesh.\n";
        buffered_ = false;
    }

    if (!buffered_)
    {
        buffer();
        return;
    }
    ebo_.buffer_sub_data(0, indices);
    vbo_.buffer_sub_data(0, vertices);
}

template <typename VertexType>
inline void Mesh<VertexType>::bind() const
{
    vao_.bind();
}

template <typename VertexType>
inline void Mesh<VertexType>::draw(GLenum draw_mode) const
{
    assert(indices_ > 0);
    glDrawElements(draw_mode, indices_, GL_UNSIGNED_INT, nullptr);
}

[[nodiscard]] BasicMesh generate_quad_mesh(float w, float h);
[[nodiscard]] BasicMesh generate_plane_mesh(float w, float d);
[[nodiscard]] BasicMesh generate_cube_mesh(const glm::vec3& size, bool repeat_texture);
[[nodiscard]] BasicMesh generate_centered_cube_mesh(const glm::vec3& size);
[[nodiscard]] BasicMesh generate_terrain_mesh(const HeightMap& height_map);
void update_terrain_mesh(BasicMesh& mesh, const HeightMap& height_map);