#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "graphics/backend/frame_buffer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/element_buffer.h"

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
    PostProcLayer();
    PostProcLayer(Shader *shader, int width = 1920, int height = 1080);
    PostProcLayer &operator=(PostProcLayer &&other);

    FrameBuffer &render(Texture &input_texture, Renderer &renderer);
    void resize(int width, int height);

    inline glm::vec2 get_size() const { return frame_buffer.get_texture_size(); }
};
}