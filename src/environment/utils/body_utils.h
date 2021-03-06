#pragma once

#include <functional>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

namespace ai
{
struct NearestLinkResult
{
    // Link attached to body
    entt::entity module_a = entt::null;
    entt::entity link_a = entt::null;

    // Link attached to free module
    entt::entity link_b = entt::null;

    float distance = 0.f;
};

void apply_color_scheme(entt::registry &registry, entt::entity body_entity);
void destroy_body(entt::registry &registry, entt::entity body_entity);
void destroy_module(entt::registry &registry, entt::entity module_entity);
NearestLinkResult find_nearest_link(entt::registry &registry, entt::entity module_entity);
unsigned int get_action_count(const entt::registry &registry, entt::entity body_entity);
unsigned int get_observation_count(const entt::registry &registry, entt::entity body_entity);
entt::entity get_module_at_point(entt::registry &registry, glm::vec2 point);
void link_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  unsigned int module_a_link_index,
                  entt::entity module_b_entity,
                  unsigned int module_b_link_index);
void link_modules(entt::registry &registry, entt::entity link_a, entt::entity link_b);
void snap_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  entt::entity link_a_entity,
                  entt::entity module_b_entity,
                  entt::entity link_b_entity);
void traverse_modules(const entt::registry &registry,
                      entt::entity body_entity,
                      std::function<void(entt::entity)> callback);
void update_body_fixtures(entt::registry &registry, entt::entity body_entity);
}