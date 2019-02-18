#include <memory>

#include <glm/glm.hpp>

#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/texture.h"
#include "graphics/element_buffer.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
Sprite::Sprite(std::string texture) : texture(texture) {}

Sprite::~Sprite() {}

glm::vec2 Sprite::get_center() const
{
    return glm::vec2(get_scale().x / 2, get_scale().y / 2);
}
}