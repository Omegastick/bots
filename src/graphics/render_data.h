#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "graphics/sprite.h"
#include "graphics/itransformable.h"

namespace SingularityTrainer
{
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

struct Line
{
    std::vector<glm::vec2> points;
    std::vector<float> widths;
    std::vector<glm::vec4> colors;
};

struct Text : ITransformable
{
    std::string text;
    std::string font;
    glm::vec4 color;
};

struct RenderData
{
    std::vector<Sprite> sprites;
    std::vector<Particle> particles;
    std::vector<Line> lines;
    std::vector<Text> texts;

    void append(const RenderData &render_data);
    void append(const std::vector<Sprite> &sprites);
    void append(const std::vector<Particle> &particles);
    void append(const std::vector<Line> &lines);
    void append(const std::vector<Text> &texts);
};
}