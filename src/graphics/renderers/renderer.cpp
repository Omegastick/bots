#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <spdlog/spdlog.h>

#include "graphics/renderers/renderer.h"
#include "graphics/renderers/batched_sprite_renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/renderers/line_renderer.h"
#include "graphics/renderers/text_renderer.h"
#include "graphics/renderers/vector_renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/frame_buffer.h"
#include "graphics/post_processing/distortion_layer.h"
#include "graphics/render_data.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "graphics/render_data.h"

namespace SingularityTrainer
{
Renderer::Renderer(int width, int height,
                   ResourceManager &resource_manager,
                   BatchedSpriteRenderer &sprite_renderer,
                   ParticleRenderer &particle_renderer,
                   LineRenderer &line_renderer,
                   TextRenderer &text_renderer,
                   VectorRenderer &vector_renderer)
    : width(width),
      height(height),
      view(glm::ortho(0, width, 0, height)),
      resource_manager(resource_manager),
      sprite_renderer(sprite_renderer),
      particle_renderer(particle_renderer),
      line_renderer(line_renderer),
      text_renderer(text_renderer),
      vector_renderer(vector_renderer),
      distortion_layer(nullptr)
{
    texture_frame_buffer = std::make_unique<FrameBuffer>();
    texture_frame_buffer->set_texture(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Renderer::resize(int width, int height)
{
    this->width = width;
    this->height = height;

    texture_frame_buffer->set_texture(width, height);
}

void Renderer::draw(const Line &line)
{
    lines.push_back(line);
}

void Renderer::draw(const std::vector<Particle> &particles)
{
    this->particles.insert(this->particles.end(), particles.begin(), particles.end());
}

void Renderer::draw(const Sprite &sprite)
{
    auto texture_iter = std::find(textures.begin(), textures.end(), sprite.texture);
    unsigned int texture_index;
    if (texture_iter == textures.end())
    {
        unsigned int texture_count = textures.size();
        textures.push_back(sprite.texture);
        texture_index = texture_count;
    }
    else
    {
        texture_index = std::distance(textures.begin(), texture_iter);
    }

    sprites.push_back(PackedSprite{texture_index,
                                   sprite.color,
                                   sprite.transform.get()});
}

void Renderer::draw(const Text &text)
{
    texts.push_back(text);
}

void Renderer::draw(const Circle &circle)
{
    vector_renderer.draw(circle);
}

void Renderer::draw(const Rectangle &rectangle)
{
    vector_renderer.draw(rectangle);
}

void Renderer::draw(const SemiCircle &semicircle)
{
    vector_renderer.draw(semicircle);
}

void Renderer::draw(const Trapezoid &trapezoid)
{
    vector_renderer.draw(trapezoid);
}

void Renderer::clear(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::begin()
{
    clear_scissor();
    glViewport(0, 0, width, height);
    clear();
    vector_renderer.begin_frame({width, height});
}

void Renderer::render(double time)
{
    if (distortion_layer != nullptr)
    {
        distortion_layer->update_mesh();
    }

    vector_renderer.end_frame();

    std::sort(sprites.begin(), sprites.end(),
              [](PackedSprite &a, PackedSprite &b) { return a.texture < b.texture; });

    unsigned int texture_index = 0;
    std::vector<glm::vec4> colors;
    std::vector<glm::mat4> transforms;
    for (const auto &sprite : sprites)
    {
        if (sprite.texture != texture_index)
        {
            if (transforms.size() > 0)
            {
                sprite_renderer.draw(textures[texture_index], transforms, colors, view);
                colors.clear();
                transforms.clear();
            }
            texture_index = sprite.texture;
        }

        colors.push_back(sprite.color);
        transforms.push_back(sprite.transform);
    }
    if (transforms.size() > 0)
    {
        sprite_renderer.draw(textures[texture_index], transforms, colors, view);
    }
    sprites.clear();

    particle_renderer.add_particles(particles, time);
    particle_renderer.draw(time, view);
    particles.clear();

    for (const auto &line : lines)
    {
        line_renderer.draw(line, view);
    }
    lines.clear();

    for (const auto &text : texts)
    {
        text_renderer.draw(text, view);
    }
    texts.clear();

    clear_scissor();
    for (const auto &post_proc_layer : post_proc_layers)
    {
        if (post_proc_layer->get_size().x != width || post_proc_layer->get_size().y != height)
        {
            post_proc_layer->resize(width, height);
        }
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    texture_frame_buffer->bind_draw();
    glViewport(0, 0, width, height);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    FrameBuffer *read_buffer = texture_frame_buffer.get();

    for (const auto &post_proc_layer : post_proc_layers)
    {
        read_buffer = &post_proc_layer->render(read_buffer->get_texture());
    }

    read_buffer->bind_read();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    post_proc_layers.clear();
    clear_distortion_layer();
}

void Renderer::push_post_proc_layer(PostProcLayer &post_proc_layer)
{
    post_proc_layers.push_back(&post_proc_layer);
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
        static_cast<int>(std::round(bottom_left.x * this->width)),
        static_cast<int>(std::round(bottom_left.y * this->height)),
        static_cast<int>(std::round((top_right.x - bottom_left.x) * this->width)),
        static_cast<int>(std::round((top_right.y - bottom_left.y) * this->height)));
}

void Renderer::clear_scissor() const
{
    glDisable(GL_SCISSOR_TEST);
}

void Renderer::clear_particles()
{
    particle_renderer.clear_particles();
}

void Renderer::apply_explosive_force(glm::vec2 position, float size, float strength)
{
    if (distortion_layer == nullptr)
    {
        return;
    }

    const glm::vec2 projected_position = view * glm::vec4(position, 0, 0);
    const glm::vec2 mesh_size = distortion_layer->get_size();
    const glm::vec2 half_mesh_size = mesh_size * 0.5f;
    auto calculated_position = projected_position * half_mesh_size;
    calculated_position += half_mesh_size;

    distortion_layer->apply_explosive_force(calculated_position, size, strength);
}

void Renderer::apply_implosive_force(glm::vec2 position, float size, float strength)
{
    if (distortion_layer == nullptr)
    {
        return;
    }

    const glm::vec2 projected_position = view * glm::vec4(position, 0, 0);
    const glm::vec2 mesh_size = distortion_layer->get_size();
    const glm::vec2 half_mesh_size = mesh_size * 0.5f;
    auto calculated_position = projected_position * half_mesh_size;
    calculated_position += half_mesh_size;

    distortion_layer->apply_implosive_force(calculated_position, size, strength);
}

void Renderer::set_distortion_layer(DistortionLayer &distortion_layer)
{
    this->distortion_layer = &distortion_layer;
    push_post_proc_layer(distortion_layer);
}

void Renderer::set_view(const glm::mat4 &view)
{
    this->view = view;
    vector_renderer.set_view(view);
}
}