#include <memory>

#include <glm/glm.hpp>

#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
Sprite::Sprite(std::string texture)
    : color(glm::vec4(1)),
      texture(texture) {}
}