#pragma once

#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>

namespace ai
{
struct RenderShapes
{
    unsigned int children = 0;
    entt::entity first = entt::null;
};

struct RenderShapeContainer
{
    entt::entity parent = entt::null;
    entt::entity next = entt::null;
    glm::vec2 pos_offset = {0.f, 0.f};
    float rot_offset = 0;
};
}