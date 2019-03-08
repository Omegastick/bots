#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

namespace SingularityTrainer
{
class VertexArray;
class VertexBuffer;
class ElementBuffer;
class ResourceManager;
class Line;

class LineRenderer
{
  private:
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    ResourceManager *resource_manager;

  public:
    LineRenderer(ResourceManager &resource_manager);

    void draw(const Line &line, const glm::mat4 &view);
};
}