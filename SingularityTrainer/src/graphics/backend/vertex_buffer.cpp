#include "glad/glad.h"

#include "vertex_buffer.h"

namespace SingularityTrainer
{
VertexBuffer::VertexBuffer(const void *data, unsigned int size, unsigned int usage_mode)
    : size(size), usage_mode(usage_mode)
{
    glGenBuffers(1, &id);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage_mode);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &id);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::clear() const
{
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage_mode);
}

void VertexBuffer::add_data(const void *data, unsigned int size, unsigned int usage_mode)
{
    bind();
    this->size = size;
    this->usage_mode = usage_mode;
    glBufferData(GL_ARRAY_BUFFER, size, data, usage_mode);
}

void VertexBuffer::add_sub_data(const void *data, unsigned int start_location, unsigned int size)
{
    bind();
    glBufferSubData(GL_ARRAY_BUFFER, start_location, size, data);
}
}