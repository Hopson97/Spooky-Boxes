#include "GBuffer.h"

#include <array>

namespace
{
    constexpr std::array<GLenum, 3> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                   GL_COLOR_ATTACHMENT2};

}

GBuffer::GBuffer(GLuint width, GLuint height)
    : g_buffer_(width, height)
{
    // Attach the position buffer, normal buffer, and the albedo specular buffer
    g_buffer_.attach_colour(TextureFormat::RGBA16F)
        .attach_colour(TextureFormat::RGBA16F)
        .attach_colour(TextureFormat::RGB8);

    glNamedFramebufferDrawBuffers(g_buffer_.id, 3, attachments.data());

    g_buffer_.attach_depth_buffer();
}

void GBuffer::bind()
{
    g_buffer_.bind();
}

void GBuffer::bind_textures()
{
    bind_position_buffer_texture(0);
    bind_normal_buffer_texture(1);
    bind_albedo_specular_buffer_texture(2);
}

void GBuffer::bind_position_buffer_texture(GLuint unit)
{
    g_buffer_.bind_colour_attachment(0, unit);
}

void GBuffer::bind_normal_buffer_texture(GLuint unit)
{
    g_buffer_.bind_colour_attachment(1, unit);
}

void GBuffer::bind_albedo_specular_buffer_texture(GLuint unit)
{
    g_buffer_.bind_colour_attachment(2, unit);
}
