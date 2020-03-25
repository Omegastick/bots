#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "graphics/colors.h"
#include "misc/transform.h"

namespace ai
{
struct Color
{
    glm::vec4 fill_color = {0.5f, 0.5f, 0.5f, 0.5f};
    glm::vec4 stroke_color = cl_white;
};

struct EcsCircle
{
    float radius = 0.f;
    float stroke_width = 0.f;
};

struct EcsRectangle
{
    float stroke_width = 0.f;
};

struct EcsSemiCircle
{
    float radius = 0.f;
    float stroke_width = 0.f;
};

struct EcsTrapezoid
{
    float top_width = 0.f;
    float bottom_width = 0.f;
    float stroke_width = 0.f;
};

struct EcsLine
{
    glm::vec2 start = {0.f, 0.f};
    glm::vec2 end = {0.f, 0.f};
    float width = 0.1f;
};
}