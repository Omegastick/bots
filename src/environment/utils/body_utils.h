#pragma once

#include <functional>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

namespace ai
{
void destroy_body(entt::registry &registry, entt::entity body_entity);
void link_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  unsigned int module_a_link_index,
                  entt::entity module_b_entity,
                  unsigned int module_b_link_index);
void snap_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  entt::entity link_a_entity,
                  entt::entity module_b_entity,
                  entt::entity link_b_entity);
void traverse_modules(entt::registry &registry,
                      entt::entity body_entity,
                      std::function<void(entt::entity)> callback);
void update_body_fixtures(entt::registry &registry, entt::entity body_entity);
}