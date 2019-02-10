#pragma once

namespace SingularityTrainer
{
class ElementBuffer
{
  private:
    unsigned int id;
    unsigned int count;

  public:
    ElementBuffer(const void *data, const unsigned int count);
    ~ElementBuffer();

    void bind() const;
    void unbind() const;

    inline unsigned int get_count() { return count; };
};
}