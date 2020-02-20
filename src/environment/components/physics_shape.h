#pragma once

#include <Box2D/Box2D.h>
#include <entt/entity/entity.hpp>

namespace ai
{
struct PhysicsShape
{
    entt::entity next = entt::null;
    b2PolygonShape shape = {};
};
}