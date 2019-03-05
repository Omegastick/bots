#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/render_data.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/backend/vertex_buffer.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
class TextRenderer
{
  private:
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    ResourceManager *resource_manager;

  public:
    TextRenderer(ResourceManager &resource_manager);

    void draw(const Text &text, const glm::mat4 &view);
};
}