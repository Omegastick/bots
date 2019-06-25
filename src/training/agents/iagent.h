#pragma once

#include <torch/torch.h>

namespace SingularityTrainer
{
class IAgent
{
  public:
    virtual ~IAgent() = 0;

    virtual std::vector<int> act(torch::Tensor observation,
                                 torch::Tensor hidden_state,
                                 torch::Tensor mask) = 0;
    virtual std::vector<std::vector<int>> act_multiple(torch::Tensor observations,
                                                       torch::Tensor hidden_states,
                                                       torch::Tensor masks) = 0;
};

inline IAgent::~IAgent() {}
}