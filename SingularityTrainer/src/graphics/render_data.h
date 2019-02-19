#pragma once

#include <vector>

#include "graphics/sprite.h"

namespace SingularityTrainer
{
struct Particle
{
    glm::vec2 start_position;
    glm::vec2 velocity;
    float start_time;
    float lifetime;
    glm::vec4 start_color;
    glm::vec4 end_color;
};

class RenderData
{
  public:
    void append(const std::vector<Sprite> &sprites, const std::vector<Particle> &particles);
    void append(const RenderData &render_data);

    std::vector<Sprite> sprites;
    std::vector<Particle> particles;
};
}