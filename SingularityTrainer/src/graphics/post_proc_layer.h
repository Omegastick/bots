#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "graphics/frame_buffer.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/element_buffer.h"

namespace SingularityTrainer
{
class Renderer;

class PostProcLayer
{
  private:
    FrameBuffer frame_buffer;
    Shader *shader;
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    int width, height;

  public:
    PostProcLayer(Shader *shader, int width = 1920, int height = 1080);

    FrameBuffer &render(Texture &input_texture, Renderer &renderer);
    void resize(int width, int height);

    inline glm::vec2 get_size() const { return frame_buffer.get_texture_size(); }
};
}