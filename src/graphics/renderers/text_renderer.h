#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

namespace ai
{
class VertexArray;
class VertexBuffer;
class ElementBuffer;
class ResourceManager;
struct Text;

class TextRenderer
{
  private:
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    ResourceManager &resource_manager;

  public:
    TextRenderer(ResourceManager &resource_manager);

    void draw(const Text &text, const glm::mat4 &view);
    void init();
};
}