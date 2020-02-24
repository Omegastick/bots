#pragma once

#include <entt/entity/entity.hpp>

namespace ai
{
struct RenderShapes
{
    unsigned int children = 0;
    entt::entity first = entt::null;
};

struct RenderShapeContainer
{
    entt::entity next = entt::null;
};
}