#pragma once

#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>

namespace ai
{
struct EcsModuleLink
{
    glm::vec2 pos_offset = {0.f, 0.f};
    float rot_offset = 0.f;
    entt::entity next = entt::null;
};
}