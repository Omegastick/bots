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

  public:
    PostProcLayer(Shader *shader);

    FrameBuffer &render(Texture &input_texture, Renderer &renderer);
};
}