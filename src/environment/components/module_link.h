#pragma once

#include <entt/entity/entity.hpp>

namespace ai
{
struct EcsModuleLink
{
    entt::entity next = entt::null;
};
}