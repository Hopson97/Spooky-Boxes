#pragma once

#include "GLResource.h"

#include "../Mesh.h"
#include <vector>

struct VertexArray : public GLResource<glCreateVertexArrays, glDeleteVertexArrays>
{
    VertexArray() = default;
    VertexArray(const BasicMesh& mesh);

    void bind() const;
    void buffer_mesh(const BasicMesh& mesh);
    void buffer_mesh(const BulletDebugMesh& mesh);

    void draw(GLenum draw_mode = GL_TRIANGLES);

  private:
    std::vector<GLBuffer> buffers_;
    GLuint indices_ = 0;
};