#pragma once

#include <array>

#include <entt/entity/entity.hpp>

namespace ai
{
struct BeginContact
{
    std::array<entt::entity, 2> entities = {entt::null, entt::null};
};

struct EndContact
{
    std::array<entt::entity, 2> entities = {entt::null, entt::null};
};
}