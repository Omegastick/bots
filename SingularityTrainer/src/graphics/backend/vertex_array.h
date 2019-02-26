#pragma once

#include <vector>

#include "glad/glad.h"

#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/vertex_buffer_layout.h"

namespace SingularityTrainer
{

class VertexArray
{
  private:
    unsigned int id;

  public:
    VertexArray();
    ~VertexArray();

    void bind() const;
    void unbind() const;
    void add_buffer(const VertexBuffer &vertex_buffer, const VertexBufferLayout &layout);

    inline int get_id() const { return id; }
};
}