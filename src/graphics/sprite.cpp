#include <memory>

#include <glm/glm.hpp>

#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
Sprite::Sprite(std::string texture) : texture(texture) {}

glm::vec2 Sprite::get_center() const
{
    return glm::vec2(get_scale().x / 2, get_scale().y / 2);
}
}