#pragma once

#include <string>

#include <glm/glm.hpp>

#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/itransformable.h"

namespace SingularityTrainer
{
class Sprite : public ITransformable
{
  private:
    std::string texture;
    glm::vec4 color;

  public:
    explicit Sprite(std::string texture);
    ~Sprite();

    glm::vec2 get_center() const;

    inline std::string get_texture() const { return texture; }
    inline void set_texture(const std::string &texture) { this->texture = texture; }
    inline glm::vec4 get_color() const { return color; }
    inline void set_color(const glm::vec4 color) { this->color = color; }
};
}