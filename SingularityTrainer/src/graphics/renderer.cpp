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
Renderer::Renderer(int width, int height) : width(width), height(height)
{
    base_frame_buffer = std::make_unique<FrameBuffer>();
    base_frame_buffer->set_render_buffer(width, height, 4);

    texture_frame_buffer = std::make_unique<FrameBuffer>();
    texture_frame_buffer->set_texture(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Renderer::~Renderer()
{
}

void Renderer::resize(int width, int height)
{
    this->width = width;
    this->height = height;

    base_frame_buffer->set_render_buffer(width, height, 4);
    texture_frame_buffer->set_texture(width, height);
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

void Renderer::begin()
{
    base_frame_buffer->bind();
    glViewport(0, 0, width, height);
    clear();
}

void Renderer::end()
{
    for (const auto &post_proc_layer : post_proc_layers)
    {
        if (post_proc_layer->get_size().x != width || post_proc_layer->get_size().y != height)
        {
            post_proc_layer->resize(width, height);
        }
    }

    base_frame_buffer->bind_read();
    texture_frame_buffer->bind_draw();
    glViewport(0, 0, width, height);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    FrameBuffer *read_buffer = texture_frame_buffer.get();

    for (const auto &post_proc_layer : post_proc_layers)
    {
        read_buffer = &post_proc_layer->render(read_buffer->get_texture(), *this);
    }

    read_buffer->bind_read();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

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