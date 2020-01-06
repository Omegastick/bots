#pragma once

#include <glad/glad.h>

namespace ai
{
class VertexBuffer
{
  private:
    unsigned int id;
    unsigned int size;
    unsigned int usage_mode;

  public:
    VertexBuffer(const void *data, unsigned int size, unsigned int usage_mode = GL_STATIC_DRAW);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;
    void clear() const;
    void add_data(const void *data, unsigned int size, unsigned int usage_mode = GL_STATIC_DRAW);
    void add_sub_data(const void *data, unsigned int start_location, unsigned int size);

    inline int get_id() const { return id; }
};
}