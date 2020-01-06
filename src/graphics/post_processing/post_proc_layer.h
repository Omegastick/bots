#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "graphics/backend/frame_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/element_buffer.h"

namespace ai
{
class Renderer;
class Shader;

class PostProcLayer
{
  protected:
    FrameBuffer frame_buffer;
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;

    int width, height;
    const Shader *shader;

  public:
    PostProcLayer(int width = 1920, int height = 1080);
    PostProcLayer(const Shader &shader, int width = 1920, int height = 1080);

    virtual FrameBuffer &render(Texture &input_texture);
    virtual void resize(int width, int height);

    inline glm::vec2 get_size() const { return frame_buffer.get_texture_size(); }
};
}