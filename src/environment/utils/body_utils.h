#pragma once

#include <functional>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

namespace ai
{
void destroy_body(entt::registry &registry, entt::entity body_entity);
entt::entity make_base_module(entt::registry &registry);
entt::entity make_body(entt::registry &registry);
entt::entity make_gun_module(entt::registry &registry);
entt::entity make_module_link(entt::registry &registry, glm::vec2 position, float rotation);
entt::entity make_thruster_module(entt::registry &registry);
void link_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  unsigned int module_a_link_index,
                  entt::entity module_b_entity,
                  unsigned int module_b_link_index);
void traverse_modules(entt::registry &registry,
                      entt::entity body_entity,
                      std::function<void(entt::entity)> callback);
void update_body_fixtures(entt::registry &registry, entt::entity body_entity);
}