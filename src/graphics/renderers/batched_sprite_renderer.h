#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>

#include "graphics/renderers/sprite_renderer.h"

namespace SingularityTrainer
{
class VertexArray;
class VertexBuffer;
class ElementBuffer;
class ResourceManager;
class Sprite;

class BatchedSpriteRenderer
{
  private:
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    ResourceManager *resource_manager;
    int max_sprites;

  public:
    BatchedSpriteRenderer(ResourceManager &resource_manager);

    void draw(const std::string &texture,
              const std::vector<glm::mat4> &transforms,
              const glm::mat4 &view);
};
}