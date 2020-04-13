#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

#include "environment/components/hill.h"

namespace ai
{
entt::entity make_hill(entt::registry &registry, glm::vec2 center, float size);
unsigned int hill_occupant_count(const EcsHill &hill);
}