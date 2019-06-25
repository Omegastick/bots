#pragma once

#include <cpprl/model/policy.h>

#include "iagent.h"

namespace SingularityTrainer
{
class NNAgent : public IAgent
{
  private:
    cpprl::Policy policy;

  public:
    NNAgent(cpprl::Policy policy);

    std::vector<int> act(torch::Tensor observation,
                         torch::Tensor hidden_state,
                         torch::Tensor mask);
    std::vector<std::vector<int>> act_multiple(torch::Tensor observations,
                                               torch::Tensor hidden_states,
                                               torch::Tensor masks);
};
}