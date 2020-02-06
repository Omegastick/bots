#pragma once

#include <string>

#include <entt/entity/entity.hpp>

namespace ai
{
struct EcsBody
{
    entt::entity base_module = entt::null;
    float hp = 0.f;
    std::string name = "";
};
}