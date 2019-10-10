#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/renderers/sprite_renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/sprite.h"
#include "misc/resource_manager.h"

namespace SingularityTrainer
{
SpriteRenderer::SpriteRenderer(ResourceManager &resource_manager) : resource_manager(&resource_manager)
{
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    vertex_array = std::make_unique<VertexArray>();

    SpriteVertex sprite_vertices[4]{
        {glm::vec2(0.0, 1.0), glm::vec2(0.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 0.0), glm::vec2(1.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)}};
    vertex_buffer = std::make_unique<VertexBuffer>(&sprite_vertices[0], 4 * sizeof(SpriteVertex));

    unsigned int sprite_indices[] = {
        0, 1, 2,
        2, 3, 0};
    element_buffer = std::make_unique<ElementBuffer>(sprite_indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);
}

void SpriteRenderer::draw(const Sprite &sprite, const glm::mat4 &view)
{
    vertex_array->bind();

    auto shader = resource_manager->shader_store.get("texture");
    shader->bind();

    auto mvp = view * sprite.get_transform();

    shader->set_uniform_mat4f("u_mvp", mvp);
    shader->set_uniform_1i("u_texture", 0);

    resource_manager->texture_store.get(sprite.get_texture())->bind();

    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);
}
}