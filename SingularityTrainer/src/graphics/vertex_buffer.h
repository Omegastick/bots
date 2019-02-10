#pragma once

namespace SingularityTrainer
{
class VertexBuffer
{
  public:
    VertexBuffer(const void *data, const unsigned int size);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;

  private:
    unsigned int id;
};
}