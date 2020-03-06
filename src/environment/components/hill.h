#pragma once

#include <array>

#include <entt/entity/entity.hpp>

namespace ai
{
struct EcsHill
{
    std::array<entt::entity, 2> occupants = {entt::null, entt::null};
    std::size_t occupant_count = 0;
};
}