#pragma once

#include <vector>
#include <exception>

#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace ai
{
struct BufferLayoutElement
{
    unsigned int type;
    unsigned int count;
    unsigned char normalized;

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
    VertexBufferLayout();

    template <typename T>
    void push(const unsigned int /*count*/)
    {
        spdlog::error("Vertex buffer layout type not implemented");
    }

    inline unsigned int get_stride() const { return stride; }
    inline const std::vector<BufferLayoutElement> &get_elements() const { return elements; }
};

template <>
inline void VertexBufferLayout::push<float>(const unsigned int count)
{
    elements.push_back({GL_FLOAT, count, GL_FALSE});
    stride += count * BufferLayoutElement::GetTypeSize(GL_FLOAT);
}

template <>
inline void VertexBufferLayout::push<unsigned int>(const unsigned int count)
{
    elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    stride += count * BufferLayoutElement::GetTypeSize(GL_UNSIGNED_INT);
}
}