#pragma once

#include <array>

#include <entt/entity/entity.hpp>

namespace ai
{
struct EcsHill
{
    std::array<std::pair<entt::entity, unsigned int>, 2> occupants = {
        std::pair<entt::entity, unsigned int>{entt::null, 0},
        std::pair<entt::entity, unsigned int>{entt::null, 0}};
};
}