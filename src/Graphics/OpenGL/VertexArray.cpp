#include "VertexArray.h"

VertexArray::VertexArray(const BasicMesh& mesh)
{
    buffer_mesh(mesh);
}

void VertexArray::bind() const
{
    assert(id);
    glBindVertexArray(id);
}

void VertexArray::buffer_mesh(const BasicMesh& mesh)
{
    assert(id);

    GLBuffer vbo;
    GLBuffer ebo;

    // Element buffer
    glNamedBufferStorage(ebo.id, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(),
                         0x0);
    glVertexArrayElementBuffer(id, ebo.id);

    // glBufferData
    // glNamedBufferStorage(vbo, points.size() * sizeof(BasicVertex), points.data(), 0x0);
    glNamedBufferStorage(vbo.id, sizeof(BasicVertex) * mesh.vertices.size(),
                         mesh.vertices.data(), GL_DYNAMIC_STORAGE_BIT);

    // Attach the vertex array to the vertex buffer and element buffer
    glVertexArrayVertexBuffer(id, 0, vbo.id, 0, sizeof(BasicVertex));

    // glEnableVertexAttribArray
    glEnableVertexArrayAttrib(id, 0);
    glEnableVertexArrayAttrib(id, 1);
    glEnableVertexArrayAttrib(id, 2);

    // glVertexAttribPointer
    glVertexArrayAttribFormat(id, 0, 3, GL_FLOAT, GL_FALSE, offsetof(BasicVertex, position));
    glVertexArrayAttribFormat(id, 1, 2, GL_FLOAT, GL_FALSE,
                              offsetof(BasicVertex, texture_coord));
    glVertexArrayAttribFormat(id, 2, 3, GL_FLOAT, GL_FALSE, offsetof(BasicVertex, normal));
    glVertexArrayAttribBinding(id, 0, 0);
    glVertexArrayAttribBinding(id, 1, 0);
    glVertexArrayAttribBinding(id, 2, 0);

    buffers_.push_back(std::move(vbo));
    buffers_.push_back(std::move(ebo));
}

void VertexArray::draw()
{
    glDrawElements(GL_TRIANGLES, indices_, GL_UNSIGNED_INT, nullptr);
}