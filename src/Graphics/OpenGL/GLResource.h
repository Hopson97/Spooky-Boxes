#pragma once

#include <glad/glad.h>
#include <memory>

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

    GLResource& operator=(GLResource&& other) noexcept  { id = other.id;  other.id = 0; }   
    GLResource(GLResource&& other) noexcept  : id  (other.id){ other.id = 0; }   

};

template<int Target>
struct GLTextureResource
{
    GLuint id;

    GLTextureResource() { glCreateTextures(Target, 1, &id); }   
    virtual ~GLTextureResource() { if(id != 0) glDeleteTextures(1, &id); }

    GLTextureResource           (const GLTextureResource& other) = delete;  
    GLTextureResource& operator=(const GLTextureResource& other) = delete;  

    GLTextureResource& operator=(GLTextureResource&& other) noexcept { id = other.id;  other.id = 0; }   
    GLTextureResource (GLTextureResource&& other) noexcept : id  (other.id){ other.id = 0; }   

    void bind(GLuint unit) { assert(id); glBindTextureUnit(unit, id); }

};
// clang-format on

struct GLVertexArray : public GLResource<&glCreateVertexArrays, &glDeleteVertexArrays>
{
    void bind()
    {
        glBindVertexArray(id);
    }
};

struct GLFramebuffer : public GLResource<&glCreateFramebuffers, &glDeleteFramebuffers>
{
    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }
};

using GLBuffer = GLResource<&glCreateBuffers, &glDeleteBuffers>;
using GLTexture2D = GLTextureResource<GL_TEXTURE_2D>;
