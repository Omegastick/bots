#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

namespace SingularityTrainer
{
class VertexArray;
class VertexBuffer;
class ElementBuffer;
class ResourceManager;
class Sprite;

struct SpriteVertex
{
    glm::vec2 position;
    glm::vec2 texture_coord;
    glm::vec4 color;
};

class SpriteRenderer
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