#pragma once

#include <memory>

#include "graphics/texture.h"

namespace SingularityTrainer
{
class FrameBuffer
{
  private:
    unsigned int id;
    unsigned int render_buffer;
    std::unique_ptr<Texture> texture;

  public:
    explicit FrameBuffer();
    ~FrameBuffer();

    void bind() const;
    void bind_read() const;
    void bind_draw() const;
    void unbind() const;
    void add_render_buffer(int width, int height, int multisampling);
    void add_texture(int width, int height);

    inline Texture &get_texture() const { return *texture; }
};
}