#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include "environment/components/physics_type.h"

namespace ai
{
void begin_bullet_contact(entt::registry &registry,
                          entt::entity bullet_entity,
                          entt::entity other_entity,
                          PhysicsType::Type other_type);
}