#pragma once

#include <cassert>
#include <glad/glad.h>

// clang-format off
/**
 * @brief Wrapper class for any OpenGL function that has a standard glCreate* or glDelete* function
 * for construction and destruction
 * 
 * @tparam CreateFunction Function to create an OpenGL object (eg &glCreateVertexArrays)
 * @tparam DeleteFunction Function to delete an OpenGL object (eg &glCreateVertexArrays) 
 */
template<auto CreateFunction, auto DeleteFunction>
struct GLResource
{
    GLuint id;

    GLResource() { (*CreateFunction)(1, &id); }
    virtual ~GLResource() { if(id != 0) (*DeleteFunction)(1, &id);   }

    GLResource(const GLResource& other)             = delete;   
    GLResource& operator=(const GLResource& other)  = delete;   

    GLResource& operator=(GLResource&& other) noexcept  { id = other.id;  other.id = 0; return *this; }   
    GLResource(GLResource&& other) noexcept  : id  (other.id){ other.id = 0; }   

};

// clang-format on

struct GLVertexArray : public GLResource<&glCreateVertexArrays, &glDeleteVertexArrays>
{
    void bind()
    {
        assert(id);
        glBindVertexArray(id);
    }
};

struct GLFramebuffer : public GLResource<&glCreateFramebuffers, &glDeleteFramebuffers>
{
    void bind()
    {
        assert(id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }
};

struct GLBuffer : public GLResource<&glCreateFramebuffers, &glDeleteFramebuffers>
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