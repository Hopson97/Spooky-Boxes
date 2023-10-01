#include "Framebuffer.h"

#include <iostream>

Framebuffer::Framebuffer(GLuint width, GLuint height)
    : width{width}
    , height{height}
{
}

Framebuffer::~Framebuffer()
{
    for (auto& renderbuffer : renderbuffers_)
    {
        glDeleteRenderbuffers(1, &renderbuffer);
    }
}

void Framebuffer::bind() const
{
    assert(id);
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::bind_colour_attachment(GLuint index, GLuint unit) const
{
    assert(index < attachments_.size());
    attachments_[index].bind(unit);
}

Framebuffer& Framebuffer::attach_colour()
{
    Texture2D& texture = attachments_.emplace_back();
    texture.create(width, height);
    glNamedFramebufferTexture(id, GL_COLOR_ATTACHMENT0, texture.id, 0);
    return *this;
}

Framebuffer& Framebuffer::attach_renderbuffer()
{
    GLuint& rbo = renderbuffers_.emplace_back();
    glCreateRenderbuffers(1, &rbo);
    glNamedRenderbufferStorage(rbo, GL_DEPTH24_STENCIL8, width, height);
    glNamedFramebufferRenderbuffer(id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    return *this;
}

bool Framebuffer::is_complete() const
{
    if (auto status =
            glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer incomplete. Status: " << status << '\n';
        return false;
    }
    return true;
}