#pragma once

#include <entt/fwd.hpp>

namespace ai
{
void module_system(entt::registry &registry);
void update_link_transforms(entt::registry &registry, entt::entity module);
}