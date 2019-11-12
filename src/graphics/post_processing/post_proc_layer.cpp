#include <memory>
#include <glm/glm.hpp>

#include "graphics/backend/frame_buffer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "graphics/post_processing/post_proc_layer.h"

namespace SingularityTrainer
{
PostProcLayer::PostProcLayer(int width, int height)
    : width(width),
      height(height)
{
    vertex_array = std::make_unique<VertexArray>();

    float vertices[] = {
        -1, 1, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        -1, -1, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0,
        1, -1, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0,
        1, 1, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

    vertex_buffer = std::make_unique<VertexBuffer>(vertices, 4 * 8 * sizeof(float));

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    element_buffer = std::make_unique<ElementBuffer>(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);

    frame_buffer.set_texture(width, height);
}

PostProcLayer::PostProcLayer(Shader &shader, int width, int height)
    : PostProcLayer(width, height)
{
    this->shader = &shader;
}

FrameBuffer &PostProcLayer::render(Texture &input_texture)
{
    glDisable(GL_BLEND);
    frame_buffer.bind();

    input_texture.bind();
    shader->set_uniform_1i("u_texture", 0);

    auto mvp = glm::mat4(1.0);
    shader->set_uniform_mat4f("u_mvp", mvp);

    glViewport(0, 0, width, height);
    vertex_array->bind();
    shader->bind();
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    glEnable(GL_BLEND);
    return frame_buffer;
}

void PostProcLayer::resize(int width, int height)
{
    this->width = width;
    this->height = height;
    frame_buffer.set_texture(width, height);
}
}