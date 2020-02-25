#include <limits>
#include <string>

#include <glad/glad.h>

#include "graphics/backend/texture.h"
#include "third_party/stb_image.h"

namespace ai
{
Texture::Texture(int width,
                 int height,
                 unsigned int internal_format,
                 unsigned int format,
                 unsigned int storage_type)
    : id(0), width(width), height(height), bpp(4)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internal_format,
                 width,
                 height,
                 0,
                 format,
                 storage_type,
                 nullptr);

    unbind();
}

Texture::Texture(int width, int height, unsigned char *data, unsigned int format)
    : id(0), width(width), height(height), bpp(4)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    unbind();
}

Texture::Texture(int width, int height, float *data)
    : id(0), width(width), height(height), bpp(4)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA32F,
                 width,
                 height,
                 0,
                 GL_RG,
                 GL_FLOAT,
                 data);
    unbind();
}

Texture::Texture(const std::string &filepath)
    : id(0), filepath(filepath), width(0), height(0), bpp(0)
{
    stbi_set_flip_vertically_on_load(1);
    auto *buffer = stbi_load(filepath.c_str(), &width, &height, &bpp, 4);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA16F,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 buffer);
    unbind();

    if (buffer)
    {
        stbi_image_free(buffer);
    }
}

Texture::Texture(Texture &&other)
    : id(other.id),
      filepath(std::move(other.filepath)),
      width(other.width),
      height(other.height),
      bpp(other.bpp)
{
    other.id = std::numeric_limits<unsigned int>::max();
}

Texture &Texture::operator=(Texture &&other)
{
    if (this != &other)
    {
        id = other.id;
        other.id = std::numeric_limits<unsigned int>::max();
        filepath = std::move(other.filepath);
        width = other.width;
        height = other.height;
        bpp = other.bpp;
    }
    return *this;
}

Texture::~Texture()
{
    if (id != std::numeric_limits<unsigned int>::max())
    {
        glDeleteTextures(1, &id);
    }
}

void Texture::bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
}