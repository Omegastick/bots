#pragma once

#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>

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
    int parent_link_index = -1;
    glm::vec2 pos_offset = {0.f, 0.f};
    float rot_offset = 0.f;
    unsigned int links = 0;
    entt::entity first_link = entt::null;
};
}