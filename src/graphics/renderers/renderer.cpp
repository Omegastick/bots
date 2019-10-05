#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "graphics/renderers/renderer.h"
#include "graphics/renderers/sprite_renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/renderers/line_renderer.h"
#include "graphics/renderers/text_renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/frame_buffer.h"
#include "graphics/render_data.h"
#include "graphics/post_proc_layer.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
Renderer::Renderer(int width, int height,
                   ResourceManager &resource_manager,
                   SpriteRenderer &sprite_renderer,
                   ParticleRenderer &particle_renderer,
                   LineRenderer &line_renderer,
                   TextRenderer &text_renderer)
    : width(width),
      height(height),
      resource_manager(resource_manager),
      sprite_renderer(sprite_renderer),
      particle_renderer(particle_renderer),
      line_renderer(line_renderer),
      text_renderer(text_renderer)
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

void Renderer::draw(const Sprite &sprite, const glm::mat4 &view)
{
    sprite_renderer.draw(sprite, view);
}

void Renderer::draw(const Text &text, const glm::mat4 &view)
{
    text_renderer.draw(text, view);
}

void Renderer::draw(RenderData &render_data, const glm::mat4 &view, double time, bool lightweight)
{
    for (const auto &sprite : render_data.sprites)
    {
        sprite_renderer.draw(sprite, view);
    }

    particle_renderer.add_particles(render_data.particles, time);
    if (!lightweight)
    {
        particle_renderer.draw(time, view);
    }

    for (const auto &line : render_data.lines)
    {
        line_renderer.draw(line, view);
    }

    for (const auto &text : render_data.texts)
    {
        text_renderer.draw(text, view);
    }
}

void Renderer::clear(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::begin()
{
    clear_scissor();
    base_frame_buffer->bind();
    glViewport(0, 0, width, height);
    clear();
}

void Renderer::end()
{
    clear_scissor();
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

void Renderer::scissor(float left, float bottom, float right, float top, const glm::mat4 &projection) const
{
    glm::vec4 bottom_left(left, bottom, 0, 0);
    glm::vec4 top_right(right, top, 0, 0);

    bottom_left = ((bottom_left * projection) + 1.f) / 2.f;
    top_right = ((top_right * projection) + 1.f) / 2.f;

    glEnable(GL_SCISSOR_TEST);
    glScissor(
        bottom_left.x * this->width,
        bottom_left.y * this->height,
        (top_right.x - bottom_left.x) * this->width,
        (top_right.y - bottom_left.y) * this->height);
}

void Renderer::clear_scissor() const
{
    glDisable(GL_SCISSOR_TEST);
}

void Renderer::clear_particles()
{
    particle_renderer.clear_particles();
}
}