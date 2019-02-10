#include <vector>

#include <glad/glad.h>

#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"

namespace SingularityTrainer
{
VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id);
    bind();
}
VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

void VertexArray::bind() const
{
    glBindVertexArray(id);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::add_buffer(const VertexBuffer &vertex_buffer, const VertexBufferLayout &layout)
{
    bind();
    vertex_buffer.bind();
    const std::vector<BufferLayoutElement> &elements = layout.get_elements();
    intptr_t offset = 0;
    for (unsigned int i = 0; i < elements.size(); ++i)
    {
        const BufferLayoutElement &element = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.get_stride(), (void *)offset);
        offset += element.count * BufferLayoutElement::GetTypeSize(element.type);
    }
    vertex_buffer.unbind();
    unbind();
}

template <>
void VertexBufferLayout::push<float>(const unsigned int count)
{
    elements.push_back({GL_FLOAT, count, GL_FALSE});
    stride += count * BufferLayoutElement::GetTypeSize(GL_FLOAT);
}

template <>
void VertexBufferLayout::push<unsigned int>(const unsigned int count)
{
    elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    stride += count * BufferLayoutElement::GetTypeSize(GL_UNSIGNED_INT);
}
}