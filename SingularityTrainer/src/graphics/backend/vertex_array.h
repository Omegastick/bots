#pragma once

#include <vector>

#include "glad/glad.h"

namespace SingularityTrainer
{
class VertexBuffer;
class VertexBufferLayout;

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