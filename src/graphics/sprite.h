#pragma once

#include <string>

#include <glm/glm.hpp>

#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/element_buffer.h"
#include "misc/transform.h"

namespace SingularityTrainer
{
class Sprite
{
  private:
    glm::vec4 color;
    std::string texture;

  public:
    Transform transform;

    explicit Sprite(std::string texture);

    inline std::string get_texture() const { return texture; }
    inline void set_texture(const std::string &texture) { this->texture = texture; }
    inline glm::vec4 get_color() const { return color; }
    inline void set_color(const glm::vec4 color) { this->color = color; }
};
}