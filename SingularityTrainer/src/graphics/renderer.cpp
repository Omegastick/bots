#include <memory>

#include <glad/glad.h>

#include "graphics/vertex_array.h"
#include "graphics/shader.h"
#include "graphics/element_buffer.h"
#include "graphics/shader.h"
#include "graphics/sprite.h"
#include "graphics/renderer.h"

namespace SingularityTrainer
{
Renderer::Renderer()
{
    base_frame_buffer = std::make_unique<FrameBuffer>();
    base_frame_buffer->add_render_buffer(1920, 1080, 4);

    texture_frame_buffer = std::make_unique<FrameBuffer>();
    texture_frame_buffer->add_texture(1920, 1080);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Renderer::~Renderer()
{
}

void Renderer::draw(const VertexArray &vertex_array, const ElementBuffer &element_buffer, const Shader &shader)
{
    vertex_array.bind();
    shader.bind();
    glDrawElements(GL_TRIANGLES, element_buffer.get_count(), GL_UNSIGNED_INT, 0);
}

void Renderer::draw(const Sprite &sprite, const Shader &shader)
{
    sprite.get_vertex_array().bind();
    shader.bind();
    glDrawElements(GL_TRIANGLES, sprite.get_element_buffer().get_count(), GL_UNSIGNED_INT, 0);
}

void Renderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::begin_frame()
{
    base_frame_buffer->bind();
    clear();
}

void Renderer::end_frame()
{
    base_frame_buffer->bind_read();
    texture_frame_buffer->bind_draw();
    glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    FrameBuffer *read_buffer = texture_frame_buffer.get();

    for (const auto &post_proc_layer : post_proc_layers)
    {
        read_buffer = &post_proc_layer->render(read_buffer->get_texture(), *this);
    }

    read_buffer->bind_read();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    post_proc_layers.clear();
}

void Renderer::push_post_proc_layer(PostProcLayer *post_proc_layer)
{
    post_proc_layers.push_back(post_proc_layer);
}

void Renderer::pop_post_proc_layer()
{
    post_proc_layers.pop_back();
}

void Renderer::clear_post_proc_stack()
{
    post_proc_layers.clear();
}
}