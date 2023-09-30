#include "Texture.h"

#include <SFML/Graphics/Image.hpp>
#include <iostream>

//=======================================
// == GLTextureResource Implementation ==
//=======================================
void GLTextureResource::set_min_filter(TextureMinFilter filter)
{
    assert(id != 0);
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filter));
}

void GLTextureResource::set_mag_filter(TextureMagFilter filter)
{
    assert(id != 0);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(filter));
}

void GLTextureResource::set_wrap_s(TextureWrap wrap)
{
    assert(id != 0);
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap));
}

void GLTextureResource::set_wrap_t(TextureWrap wrap)
{
    assert(id != 0);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap));
}

//===============================
// == Texture2D Implementation ==
//================================
Texture2D::Texture2D()
    : GLTextureResource(GL_TEXTURE_2D)
{
}

GLuint Texture2D::create(GLsizei width, GLsizei height, GLsizei levels,
                         TextureFormat internal_format)
{
    glTextureStorage2D(id, levels, static_cast<GLenum>(internal_format), width, height);

    set_min_filter(TextureMinFilter::Linear);
    set_mag_filter(TextureMagFilter::Linear);

    return id;
}

bool Texture2D::load_from_file(const std::filesystem::path& path, GLsizei levels,
                               bool flip_vertically, bool flip_horizontally,
                               TextureInternalFormat internal_format, TextureFormat format)
{
    std::cout << "Loading texture " << path << '\n';
    sf::Image image;

    // clang-format off
    if (!image.loadFromFile(path.string())) return false;

    if (flip_vertically)    image.flipVertically();
    if (flip_horizontally)  image.flipHorizontally();
    // clang-format on

    // Get shorthand image data
    auto w = image.getSize().x;
    auto h = image.getSize().y;
    auto data = image.getPixelsPtr();

    // Allocate the storage
    glTextureStorage2D(id, levels, static_cast<GLenum>(format), w, h);

    // Uplodad the pixels
    glTextureSubImage2D(id, 0, 0, 0, w, h, static_cast<GLenum>(internal_format),
                        GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(id);

    // Set some default wrapping
    set_min_filter(TextureMinFilter::LinearMipmapLinear);
    set_mag_filter(TextureMagFilter::Linear);
    set_wrap_s(TextureWrap::Repeat);
    set_wrap_t(TextureWrap::Repeat);
    return true;
}