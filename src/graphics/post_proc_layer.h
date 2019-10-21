#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "graphics/backend/frame_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/element_buffer.h"

namespace SingularityTrainer
{
class Renderer;
class Shader;

class PostProcLayer
{
  private:
    FrameBuffer frame_buffer;
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;

  protected:
    int width, height;
    Shader &shader;

  public:
    PostProcLayer(Shader &shader, int width = 1920, int height = 1080);
    PostProcLayer &operator=(PostProcLayer &&other);

    virtual FrameBuffer &render(Texture &input_texture);
    void resize(int width, int height);

    inline glm::vec2 get_size() const { return frame_buffer.get_texture_size(); }
};
}