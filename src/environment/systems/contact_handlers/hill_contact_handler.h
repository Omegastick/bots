#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include "environment/components/physics_type.h"

namespace ai
{
void begin_hill_contact(entt::registry &registry,
                        entt::entity hill_entity,
                        entt::entity other_entity,
                        PhysicsType::Type other_type);

void end_hill_contact(entt::registry &registry,
                      entt::entity hill_entity,
                      entt::entity other_entity,
                      PhysicsType::Type other_type);
}