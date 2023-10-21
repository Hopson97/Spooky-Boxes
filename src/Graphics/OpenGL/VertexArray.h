#pragma once

#include <vector>

#include "../Mesh.h"
#include "GLResource.h"


struct BufferObject : public GLResource<glCreateBuffers, glDeleteBuffers>
{
    template<typename T> 
    void buffer_data(const std::vector<T>& data)
    {
        glNamedBufferStorage(id, sizeof(data[0]) * data.size(), data.data(),
                                 GL_DYNAMIC_STORAGE_BIT);
    }
};

struct VertexArray : public GLResource<glCreateVertexArrays, glDeleteVertexArrays>
{
    VertexArray() = default;
    VertexArray(const BasicMesh& mesh);

    void bind() const;
    void buffer_mesh(const BasicMesh& mesh);
    void buffer_mesh(const DebugMesh& mesh);

    void add_attribute(const BufferObject& vbo, GLsizei stride, GLint size, GLenum type,
                       GLuint offset);

    void draw(GLenum draw_mode = GL_TRIANGLES);

  private:
    std::vector<BufferObject> buffers_;
    GLuint indices_ = 0;
    GLuint attribs_ = 0;
};