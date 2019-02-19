#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "graphics/backend/texture.h"

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
    void set_render_buffer(int width, int height, int multisampling);
    void set_texture(int width, int height);

    inline Texture &get_texture() const { return *texture; }
    inline glm::vec2 get_texture_size() const { return glm::vec2(texture->get_width(), texture->get_height()); }
};
}