#pragma once

namespace SingularityTrainer
{
class VertexBuffer
{
  public:
    VertexBuffer(const void *data, const unsigned int size);
    ~VertexBuffer();

    const void bind();
    const void unbind();

  private:
    unsigned int id;
};
}