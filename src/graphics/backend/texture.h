#pragma once

#include <string>

#include <glad/glad.h>

namespace SingularityTrainer
{
class Texture
{
  private:
    unsigned int id;
    std::string filepath;
    // unsigned char *buffer;
    int width, height, bpp;

  public:
    Texture(int width, int height);
    Texture(int width, int height, unsigned char *data, unsigned int format = GL_RED);
    Texture(int width, int height, float *data);
    explicit Texture(const std::string &filepath);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    inline int get_width() const { return width; }
    inline int get_height() const { return height; }
    inline unsigned int get_id() const { return id; }
};
}