#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "graphics/colors.h"

namespace ai
{
struct ParticleEmitter
{
    glm::vec2 position = {0.f, 0.f};
    unsigned int particle_count = 0;
    glm::vec4 start_color = cl_white;
    glm::vec4 end_color = cl_white;
    float lifetime = 0.75f;
    float size = 0.03f;
    bool directional = false;
    float direction = 0.f;
    float spread = 1.f;
    bool loop = false;
};
}