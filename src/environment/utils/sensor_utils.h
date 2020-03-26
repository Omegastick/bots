#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace ai
{
void resize_sensor(entt::registry &registry, entt::entity sensor_entity, unsigned int size);
}