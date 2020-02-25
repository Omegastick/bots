#pragma once

#include <string>

#include <glad/glad.h>

namespace ai
{
class Texture
{
  private:
    unsigned int id;
    std::string filepath;
    int width, height, bpp;

  public:
    Texture(int width,
            int height,
            unsigned int internal_format = GL_RGBA16F,
            unsigned int format = GL_RGBA,
            unsigned int storage_type = GL_UNSIGNED_BYTE);
    Texture(int width, int height, unsigned char *data, unsigned int format = GL_RED);
    Texture(int width, int height, float *data);
    explicit Texture(const std::string &filepath);

    Texture(Texture &&other);
    Texture &operator=(Texture &&other);
    Texture &operator=(const Texture &other) = delete;
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    inline int get_width() const { return width; }
    inline int get_height() const { return height; }
    inline unsigned int get_id() const { return id; }
};
}