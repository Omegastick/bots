#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "graphics/colors.h"
#include "misc/transform.h"

namespace ai
{
struct Line
{
    glm::vec2 start = {0.f, 0.f};
    glm::vec2 end = {0.f, 0.f};
    glm::vec4 color = cl_white;
    float width = 0.1f;
    Transform transform = Transform();
};

struct Particle
{
    glm::vec2 start_position;
    glm::vec2 velocity;
    float start_time_offset;
    float lifetime;
    float size;
    glm::vec4 start_color = cl_white;
    glm::vec4 end_color = cl_white;
};

struct Sprite
{
    glm::vec4 color = cl_white;
    std::string texture;
    Transform transform = Transform();
};

struct Text
{
    glm::vec4 color = cl_white;
    std::string font;
    std::string text;
    Transform transform = Transform();
};

// Vector graphics
struct Circle
{
    float radius;
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
    Transform transform = Transform();
};

struct Rectangle
{
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
    Transform transform = Transform();
};

struct SemiCircle
{
    float radius;
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
    Transform transform = Transform();
};

struct Trapezoid
{
    float top_width;
    float bottom_width;
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
    Transform transform = Transform();
};
}