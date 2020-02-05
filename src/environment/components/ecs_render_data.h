#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "graphics/colors.h"
#include "misc/transform.h"

namespace ai
{
struct EcsCircle
{
    float radius;
    glm::vec4 fill_color;
    glm::vec4 stroke_color;
    float stroke_width;
};

struct EcsRectangle
{
    glm::vec4 fill_color;
    glm::vec4 stroke_color;
    float stroke_width;
};

struct EcsSemiCircle
{
    float radius;
    glm::vec4 fill_color;
    glm::vec4 stroke_color;
    float stroke_width;
};

struct EcsTrapezoid
{
    float top_width;
    float bottom_width;
    glm::vec4 fill_color;
    glm::vec4 stroke_color;
    float stroke_width;
};
}