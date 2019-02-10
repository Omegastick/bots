#include "glad/glad.h"

#include "element_buffer.h"

namespace SingularityTrainer
{
ElementBuffer::ElementBuffer(const void *data, const unsigned int count) : count(count)
{
    glGenBuffers(1, &id);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

ElementBuffer::~ElementBuffer()
{
    glDeleteBuffers(1, &id);
}

const void ElementBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

const void ElementBuffer::unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
}