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
    float radius = 0.f;
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
};

struct EcsRectangle
{
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
};

struct EcsSemiCircle
{
    float radius = 0.f;
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
};

struct EcsTrapezoid
{
    float top_width = 0.f;
    float bottom_width = 0.f;
    glm::vec4 fill_color = cl_white;
    glm::vec4 stroke_color = cl_white;
    float stroke_width = 0.f;
};
}