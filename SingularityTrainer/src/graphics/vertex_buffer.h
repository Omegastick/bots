#pragma once

namespace SingularityTrainer
{
class VertexBuffer
{
  public:
    VertexBuffer(const void *data, const unsigned int size);
    ~VertexBuffer();

    void bind();
    void unbind();

  private:
    unsigned int id;
};
}