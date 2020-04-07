#pragma once

#include <vector>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <torch/types.h>

namespace ai
{
void action_system(entt::registry &registry,
                   const std::vector<torch::Tensor> &actions,
                   entt::entity *bodies,
                   std::size_t body_count);
}