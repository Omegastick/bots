#pragma once

#include <string>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <nlohmann/json.hpp>

namespace ai
{
void deserialize_body(entt::registry &registry, const nlohmann::json &json);
nlohmann::json serialize_body(entt::registry &registry,
                              entt::entity body_entity,
                              const std::string &name = "");
}