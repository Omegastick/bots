#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace ai
{
entt::entity make_wall(entt::registry &registry, glm::vec2 center, glm::vec2 size, float angle);
}