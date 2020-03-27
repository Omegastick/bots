#pragma once

#include <string>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <glm/vec2.hpp>

namespace ai
{
entt::entity make_base_module(entt::registry &registry);
entt::entity make_body(entt::registry &registry);
entt::entity make_gun_module(entt::registry &registry);
entt::entity make_laser_sensor_module(entt::registry &registry);
entt::entity make_module(entt::registry &registry, const std::string &type);
entt::entity make_module_link(entt::registry &registry, glm::vec2 position, float rotation);
entt::entity make_thruster_module(entt::registry &registry);
}