#pragma once

#include <array>
#include <limits>

#include <glm/vec2.hpp>

namespace ai
{
struct Trail
{
    float width = 1.f;
    static constexpr glm::vec2 vec_min = {-std::numeric_limits<float>::infinity(),
                                          -std::numeric_limits<float>::infinity()};
    std::array<glm::vec2, 3> previous_positions = {vec_min, vec_min, vec_min};
};
}