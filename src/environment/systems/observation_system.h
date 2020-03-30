#pragma once

#include <vector>

#include <entt/entity/registry.hpp>
#include <torch/types.h>

namespace ai
{
std::vector<torch::Tensor> observation_system(entt::registry &registry);
}