#pragma once

#include "graphics/vertex_array.h"
#include "graphics/shader.h"
#include "graphics/element_buffer.h"
#include "graphics/shader.h"

namespace SingularityTrainer
{
class Renderer
{
  public:
    void draw(const VertexArray &vertex_array, const ElementBuffer &element_buffer, const Shader &shader) const;
    void clear() const;
};
}