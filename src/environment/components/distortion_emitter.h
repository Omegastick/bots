#pragma once

#include <glm/vec2.hpp>

namespace ai
{
struct DistortionEmitter
{
    glm::vec2 position = {0.f, 0.f};
    float strength = 0.f;
    float size = 0.f;
    bool explosive = true;
    bool loop = false;
};
}