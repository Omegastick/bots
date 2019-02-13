#include <memory>

#include <glm/glm.hpp>

#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/texture.h"
#include "graphics/element_buffer.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
Sprite::Sprite(const std::shared_ptr<Texture> &texture) : texture(texture)
{
    vertex_array = std::make_unique<VertexArray>();

    Vertex vertices[4]{
        {glm::vec2(0.0, 1.0), glm::vec2(0.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 0.0), glm::vec2(1.0, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0)},
        {glm::vec2(1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)}};
    vertex_buffer = std::make_unique<VertexBuffer>(&vertices[0], 4 * sizeof(Vertex));

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};
    element_buffer = std::make_unique<ElementBuffer>(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);
}

Sprite::~Sprite() {}

glm::vec2 Sprite::get_center() const
{
    return glm::vec2(get_scale().x / 2, get_scale().y / 2);
}
}