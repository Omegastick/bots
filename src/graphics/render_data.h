#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "misc/transform.h"

namespace SingularityTrainer
{
struct Line
{
    std::vector<glm::vec2> points;
    std::vector<float> widths;
    std::vector<glm::vec4> colors;
};

struct Particle
{
    glm::vec2 start_position;
    glm::vec2 velocity;
    float start_time_offset;
    float lifetime;
    float size;
    glm::vec4 start_color;
    glm::vec4 end_color;
};

struct Sprite
{
    glm::vec4 color;
    std::string texture;
    Transform transform;
};

struct Text
{
    glm::vec4 color;
    std::string font;
    std::string text;
    Transform transform;
};
}