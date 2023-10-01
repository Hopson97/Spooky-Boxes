#pragma once

#include "GLResource.h"
#include "Texture.h"

#include <unordered_map>

struct Framebuffer : public GLResource<glCreateFramebuffers, glDeleteFramebuffers>
{
    Framebuffer(GLuint width, GLuint height);
    ~Framebuffer();

    void bind() const;
    void bind_colour_attachment(GLuint index, GLuint unit) const;

    Framebuffer& attach_colour();
    Framebuffer& attach_renderbuffer();

    bool is_complete() const;

  private:
    std::vector<Texture2D> attachments_;
    std::vector<GLuint> renderbuffers_;
    GLuint width = 0;
    GLuint height = 0;
};
