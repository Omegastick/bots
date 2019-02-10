#pragma once

#include <vector>

#include "glad/glad.h"

#include "graphics/vertex_buffer.h"

namespace SingularityTrainer
{
struct BufferLayoutElement
{
    unsigned int type;
    unsigned int count;
    int normalized;

    static unsigned int GetTypeSize(const unsigned int type)
    {
        switch (type)
        {
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        }
        return 0;
    }
};

class VertexBufferLayout
{
  private:
    unsigned int stride;
    std::vector<BufferLayoutElement> elements;

  public:
    VertexBufferLayout() : stride(0) {}
    ~VertexBufferLayout(){};

    template <typename T>
    void push(const unsigned int count)
    {
        throw "Type not implemented";
    }

    inline unsigned int get_stride() const { return stride; }
    inline const std::vector<BufferLayoutElement> &get_elements() const { return elements; }
};

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
};
}