#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "graphics/frame_buffer.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
class Renderer;

class PostProcLayer
{
  private:
    FrameBuffer frame_buffer;
    Shader *shader;
    std::unique_ptr<Sprite> sprite;
    glm::mat4 projection;

  public:
    PostProcLayer(Shader *shader, const glm::mat4 &projection);

    FrameBuffer &render(Texture &input_texture, Renderer &renderer);
};
}