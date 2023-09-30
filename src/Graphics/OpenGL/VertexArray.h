#pragma once

#include "GLResource.h"

#include "../Mesh.h"
#include <vector>

struct VAODrawData
{
    GLuint id;
    GLsizei indices_;
};

struct GLVertexArray : public GLResource<&glCreateVertexArrays, &glDeleteVertexArrays>
{
    GLVertexArray() = default;
    GLVertexArray(const BasicMesh& mesh);

    void bind() const;
    void buffer_mesh(const BasicMesh& mesh);

    void draw();

  private:
    std::vector<GLBuffer> buffers_;
    GLuint indices_;
};