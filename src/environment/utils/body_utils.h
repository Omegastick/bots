#pragma once

#include <entt/fwd.hpp>
#include <glm/vec2.hpp>

namespace ai
{
entt::entity create_base_module(entt::registry &registry);
entt::entity create_body(entt::registry &registry);
entt::entity create_module_link(entt::registry &registry, glm::vec2 position, float rotation);
void link_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  unsigned int module_a_link_index,
                  entt::entity module_b_entity,
                  unsigned int module_b_link_index);
void update_body_fixtures(entt::registry &registry, entt::entity body_entity);
}