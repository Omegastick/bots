#pragma once

#include <entt/entity/entity.hpp>

namespace ai
{
struct PhysicsShapes
{
    unsigned int count = 0;
    entt::entity first = entt::null;
};
}