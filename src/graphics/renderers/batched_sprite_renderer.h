#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

#include "graphics/renderers/sprite_renderer.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"

namespace SingularityTrainer
{
class ResourceManager;

class BatchedSpriteRenderer
{
  private:
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    ResourceManager *resource_manager;
    int max_sprites;
    std::vector<SpriteVertex> transformed_vertices;

  public:
    BatchedSpriteRenderer(ResourceManager &resource_manager);

    void draw(const std::string &texture,
              const std::vector<glm::mat4> &transforms,
              const std::vector<glm::vec4> &colors,
              const glm::mat4 &view);
};
}