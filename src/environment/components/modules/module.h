#pragma once

#include <entt/entity/entity.hpp>

namespace ai
{
struct EcsModule
{
    entt::entity body = entt::null;
    unsigned int children = 0;
    entt::entity first = entt::null;
    entt::entity prev = entt::null;
    entt::entity next = entt::null;
    entt::entity parent = entt::null;
};
}