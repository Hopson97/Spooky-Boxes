#include "VertexArray.h"

// VertexArray::VertexArray(const BasicMesh& mesh)
// {
//     buffer_mesh(mesh);
// }

void VertexArray::bind() const
{
    assert(id);
    glBindVertexArray(id);
}

// void VertexArray::buffer_mesh(const BasicMesh& mesh)
// {
//     assert(id);

//     // Attach EBO
//     BufferObject ebo;
//     ebo.buffer_data(mesh.indices);
//     glVertexArrayElementBuffer(id, ebo.id);
//     indices_ = static_cast<GLuint>(mesh.indices.size());

//     // Attach VBO
//     BufferObject vbo;
//     vbo.buffer_data(mesh.vertices);
//     add_attribute(vbo, sizeof(BasicVertex), 3, GL_FLOAT, offsetof(BasicVertex, position));
//     add_attribute(vbo, sizeof(BasicVertex), 2, GL_FLOAT, offsetof(BasicVertex,
//     texture_coord)); add_attribute(vbo, sizeof(BasicVertex), 3, GL_FLOAT,
//     offsetof(BasicVertex, normal));

//     buffers_.push_back(std::move(vbo));
//     buffers_.push_back(std::move(ebo));
// }

// void VertexArray::buffer_mesh(const DebugMesh& mesh)
// {
//     // Attach EBO
//     BufferObject ebo;
//     ebo.buffer_data(mesh.indices);
//     glVertexArrayElementBuffer(id, ebo.id);
//     indices_ = static_cast<GLuint>(mesh.indices.size());

//     // Attach VBO
//     BufferObject vbo;
//     vbo.buffer_data(mesh.vertices);
//     add_attribute(vbo, sizeof(DebugVertex), 3, GL_FLOAT, offsetof(DebugVertex, position));
//     add_attribute(vbo, sizeof(DebugVertex), 3, GL_FLOAT, offsetof(DebugVertex, colour));

//     buffers_.push_back(std::move(vbo));
//     buffers_.push_back(std::move(ebo));
// }

void VertexArray::add_attribute(const BufferObject& vbo, GLsizei stride, GLint size,
                                GLenum type, GLuint offset)
{
    glEnableVertexArrayAttrib(id, attribs_);
    glVertexArrayVertexBuffer(id, attribs_, vbo.id, 0, stride);
    glVertexArrayAttribFormat(id, attribs_, size, type, GL_FALSE, offset);
    glVertexArrayAttribBinding(id, attribs_, 0);
    attribs_++;
}
