#pragma once

#include "GLResource.h"

#include "../Mesh.h"
#include <vector>

struct VAODrawData
{
    GLuint id;
    GLsizei indices_;
};

struct VertexArray : public GLResource<&glCreateVertexArrays, &glDeleteVertexArrays>
{
    VertexArray() = default;
    VertexArray(const BasicMesh& mesh);

    void bind() const;
    void buffer_mesh(const BasicMesh& mesh);

    void draw();

  private:
    std::vector<GLBuffer> buffers_;
    GLuint indices_;
};