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
Renderer::Renderer(int width, int height, ResourceManager &resource_manager)
    : width(width), height(height), resource_manager(&resource_manager)
{
    base_frame_buffer = std::make_unique<FrameBuffer>();
    base_frame_buffer->set_render_buffer(width, height, 4);

    texture_frame_buffer = std::make_unique<FrameBuffer>();
    texture_frame_buffer->set_texture(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    sprite_vertex_array = std::make_unique<VertexArray>();

    SpriteVertex sprite_vertices[4]{
        {glm::vec2(0.0, 1.0), glm::vec2(0.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 0.0), glm::vec2(1.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)}};
    sprite_vertex_buffer = std::make_unique<VertexBuffer>(&sprite_vertices[0], 4 * sizeof(SpriteVertex));

    unsigned int sprite_indices[] = {
        0, 1, 2,
        2, 3, 0};
    sprite_element_buffer = std::make_unique<ElementBuffer>(sprite_indices, 6);

    VertexBufferLayout sprite_layout;
    sprite_layout.push<float>(2);
    sprite_layout.push<float>(2);
    sprite_layout.push<float>(4);
    sprite_vertex_array->add_buffer(*sprite_vertex_buffer, sprite_layout);
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

void Renderer::draw(const Sprite &sprite, const glm::mat4 &transform)
{
    sprite_vertex_array->bind();
    auto shader = resource_manager->shader_store.get("texture");
    shader->bind();
    auto mvp = transform * sprite.get_transform();
    shader->set_uniform_mat4f("u_mvp", mvp);
    shader->set_uniform_1i("u_texture", 0);
    resource_manager->texture_store.get(sprite.get_texture())->bind();
    glDrawElements(GL_TRIANGLES, sprite_element_buffer->get_count(), GL_UNSIGNED_INT, 0);
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