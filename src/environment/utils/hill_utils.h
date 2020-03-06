#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

namespace ai
{
entt::entity make_hill(entt::registry &registry, glm::vec2 center, float size);
}