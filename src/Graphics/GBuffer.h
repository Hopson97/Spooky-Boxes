#pragma once

#include "OpenGL/Framebuffer.h"

class GBuffer
{
  public:
    GBuffer(GLuint width, GLuint height);
    void bind();

    void bind_textures();

    void bind_position_buffer_texture(GLuint unit);
    void bind_normal_buffer_texture(GLuint unit);
    void bind_albedo_specular_buffer_texture(GLuint unit);

  private:
    Framebuffer g_buffer_;
};