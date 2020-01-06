#pragma once

namespace ai
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

    void set_data(const void *data, const unsigned int count);

    inline unsigned int get_count() const { return count; };
    inline unsigned int get_id() const { return id; };
};
}