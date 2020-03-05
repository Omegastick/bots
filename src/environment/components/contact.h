#pragma once

#include <entt/entity/entity.hpp>

namespace ai
{
struct BeginContact
{
    entt::entity entity_1 = entt::null;
    entt::entity entity_2 = entt::null;
};

struct EndContact
{
    entt::entity entity_1 = entt::null;
    entt::entity entity_2 = entt::null;
};
}