#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/backend/element_buffer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"

namespace ai
{
class ResourceManager;
struct Sprite;

struct SpriteVertex
{
    glm::vec2 position;
    glm::vec2 texture_coord;
    glm::vec4 color;
};

struct SpriteRenderer
{
  private:
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    ResourceManager *resource_manager;

  public:
    SpriteRenderer(ResourceManager &resource_manager);

    void draw(const Sprite &sprite, const glm::mat4 &view);
};
}