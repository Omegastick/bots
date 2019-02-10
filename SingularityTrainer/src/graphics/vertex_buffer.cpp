#include "glad/glad.h"

#include "vertex_buffer.h"

namespace SingularityTrainer
{
VertexBuffer::VertexBuffer(const void *data, const unsigned int size)
{
    glGenBuffers(1, &id);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &id);
}

const void VertexBuffer::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

const void VertexBuffer::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
}