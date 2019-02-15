#include <glad/glad.h>

#include "graphics/vertex_buffer_layout.h"

namespace SingularityTrainer
{
VertexBufferLayout::VertexBufferLayout() : stride(0) {}

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