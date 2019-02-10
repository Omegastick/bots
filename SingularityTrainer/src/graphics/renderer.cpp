#include <glad/glad.h>

#include "graphics/vertex_array.h"
#include "graphics/shader.h"
#include "graphics/element_buffer.h"
#include "graphics/shader.h"
#include "graphics/renderer.h"

namespace SingularityTrainer
{
void Renderer::draw(const VertexArray &vertex_array, const ElementBuffer &element_buffer, const Shader &shader) const
{
    vertex_array.bind();
    shader.bind();
    glDrawElements(GL_TRIANGLES, element_buffer.get_count(), GL_UNSIGNED_INT, 0);
}

void Renderer::clear() const
{
    glClear(GL_COLOR_BUFFER_BIT);
}
}