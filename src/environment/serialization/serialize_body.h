#pragma once

#include <string>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <nlohmann/json.hpp>

namespace ai
{
entt::entity deserialize_body(entt::registry &registry, const nlohmann::json &json);
entt::entity deserialize_module(entt::registry &registry, const nlohmann::json &json);
nlohmann::json serialize_body(const entt::registry &registry, entt::entity body_entity);
nlohmann::json serialize_module(const entt::registry &registry,
                                entt::entity module_entity);
nlohmann::json default_body();
}