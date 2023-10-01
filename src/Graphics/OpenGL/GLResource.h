#pragma once

#include <cassert>
#include <iostream>

#include <glad/glad.h>

// clang-format off
/**
 * @brief Wrapper class for any OpenGL function that has a standard glCreate* or glDelete* function
 * for construction and destruction
 * 
 * @tparam CreateFunction Function to create an OpenGL object (eg &glCreateVertexArrays)
 * @tparam DeleteFunction Function to delete an OpenGL object (eg &glCreateVertexArrays) 
 */
template<auto& CreateFunction, auto& DeleteFunction>
struct GLResource
{
    GLuint id = 0;

    GLResource() { (*CreateFunction)(1, &id);}
    virtual ~GLResource() { destroy(); }

    GLResource(const GLResource& other)             = delete;   
    GLResource& operator=(const GLResource& other)  = delete;   

    GLResource& operator=(GLResource&& other) noexcept  { destroy(); id = other.id;  other.id = 0; return *this; }   
    GLResource(GLResource&& other) noexcept  : id  (other.id){ other.id = 0; }

    void destroy() { if(id != 0) {(*DeleteFunction)(1, &id); id = 0; } }
};

// clang-format on
struct GLBuffer : public GLResource<glCreateBuffers, glDeleteBuffers>
{
    enum class Target
    {
        ArrayBuffer = GL_ARRAY_BUFFER
    };

    void bind(Target target)
    {
        assert(id);
        glBindBuffer(static_cast<GLenum>(target), id);
    }
};