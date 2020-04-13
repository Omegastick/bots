#pragma once

#include <entt/entity/registry.hpp>

namespace ai
{
void body_death_system(entt::registry &registry, entt::entity *bodies, std::size_t body_count);
}