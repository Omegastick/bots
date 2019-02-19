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

struct RenderData
{
    std::vector<Sprite> sprites;
    std::vector<Particle> particles;
};
}