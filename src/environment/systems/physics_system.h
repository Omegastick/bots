#pragma once

#include <entt/fwd.hpp>

namespace ai
{
void init_physics(entt::registry &registry);
void physics_system(entt::registry &registry, double delta_time);
}