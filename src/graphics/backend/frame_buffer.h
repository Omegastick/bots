#pragma once

#include <memory>

#include <glm/glm.hpp>

namespace ai
{
class Texture;

class FrameBuffer
{
  private:
    unsigned int id;
    unsigned int render_buffer;
    std::unique_ptr<Texture> texture;
    std::unique_ptr<Texture> stencil_buffer;

  public:
    explicit FrameBuffer();
    ~FrameBuffer();
    FrameBuffer &operator=(FrameBuffer &&other);

    void bind() const;
    void bind_read() const;
    void bind_draw() const;
    void unbind() const;
    void set_render_buffer(int width, int height, int multisampling);
    void set_texture(int width, int height, bool use_stencil_buffer = false);

    inline Texture &get_texture() const { return *texture; }
    glm::vec2 get_texture_size() const;
};
}