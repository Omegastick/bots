#pragma once

#include <tuple>

#include <torch/torch.h>

namespace SingularityTrainer
{
// Actions, Hiden states
typedef std::tuple<torch::Tensor, torch::Tensor> ActResult;

class IAgent
{
  public:
    virtual ~IAgent() = 0;

    virtual ActResult act(torch::Tensor observations,
                          torch::Tensor hidden_states,
                          torch::Tensor masks) = 0;
    virtual int get_hidden_state_size() = 0;
};

inline IAgent::~IAgent() {}
}