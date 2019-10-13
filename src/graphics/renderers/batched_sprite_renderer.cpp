#include <memory>

#include <easy/profiler.h>
#include <fmt/ostream.h>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>

#include "batched_sprite_renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/sprite.h"
#include "misc/resource_manager.h"

namespace SingularityTrainer
{
BatchedSpriteRenderer::BatchedSpriteRenderer(ResourceManager &resource_manager)
    : resource_manager(&resource_manager),
      max_sprites(100000),
      transformed_vertices(max_sprites * 4)
{

    resource_manager.load_shader("batched_texture",
                                 "shaders/texture_batched.vert",
                                 "shaders/texture.frag");
    vertex_array = std::make_unique<VertexArray>();

    SpriteVertex sprite_vertices[4]{
        {glm::vec2(0.0, 1.0), glm::vec2(0.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 0.0), glm::vec2(1.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)}};
    vertex_buffer = std::make_unique<VertexBuffer>(&sprite_vertices[0], 4 * sizeof(SpriteVertex));

    std::vector<unsigned int> sprite_indices;
    for (int i = 0; i < max_sprites; ++i)
    {
        unsigned int sprite_number = i * 4;
        sprite_indices.push_back(0 + sprite_number);
        sprite_indices.push_back(1 + sprite_number);
        sprite_indices.push_back(2 + sprite_number);
        sprite_indices.push_back(2 + sprite_number);
        sprite_indices.push_back(3 + sprite_number);
        sprite_indices.push_back(0 + sprite_number);
    }
    element_buffer = std::make_unique<ElementBuffer>(sprite_indices.data(), max_sprites * 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);
}

void BatchedSpriteRenderer::draw(const std::string &texture,
                                 const std::vector<glm::mat4> &transforms,
                                 const glm::mat4 &view)
{
    vertex_array->bind();

    auto shader = resource_manager->shader_store.get("batched_texture");
    shader->bind();

    for (unsigned int i = 0; i < transforms.size(); ++i)
    {
        const auto &transform = transforms[i];

        glm::vec4 transformed_position = transform * glm::vec4(0.0, 1.0, 1.0, 1.0);
        transformed_vertices[i * 4] = {glm::vec2(transformed_position.x,
                                                 transformed_position.y),
                                       glm::vec2(0.0, 1.0),
                                       glm::vec4(1.0, 1.0, 1.0, 1.0)};

        transformed_position = transform * glm::vec4(0.0, 0.0, 1.0, 1.0);
        transformed_vertices[i * 4 + 1] = {glm::vec2(transformed_position.x,
                                                     transformed_position.y),
                                           glm::vec2(0.0, 0.0),
                                           glm::vec4(1.0, 1.0, 1.0, 1.0)};

        transformed_position = transform * glm::vec4(1.0, 0.0, 1.0, 1.0);
        transformed_vertices[i * 4 + 2] = {glm::vec2(transformed_position.x,
                                                     transformed_position.y),
                                           glm::vec2(1.0, 0.0),
                                           glm::vec4(1.0, 1.0, 1.0, 1.0)};

        transformed_position = transform * glm::vec4(1.0, 1.0, 1.0, 1.0);
        transformed_vertices[i * 4 + 3] = {glm::vec2(transformed_position.x,
                                                     transformed_position.y),
                                           glm::vec2(1.0, 1.0),
                                           glm::vec4(1.0, 1.0, 1.0, 1.0)};
    }

    vertex_buffer->add_data(transformed_vertices.data(),
                            sizeof(SpriteVertex) * (transforms.size() * 4),
                            GL_DYNAMIC_DRAW);

    shader->set_uniform_mat4f("u_view", view);
    shader->set_uniform_1i("u_texture", 0);

    resource_manager->texture_store.get(texture)->bind();

    glDrawElements(GL_TRIANGLES, transforms.size() * 6, GL_UNSIGNED_INT, 0);
}
}