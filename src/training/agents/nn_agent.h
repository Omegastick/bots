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

    ActResult act(torch::Tensor observations,
                  torch::Tensor hidden_states,
                  torch::Tensor masks);

    inline int get_hidden_state_size() { return policy->get_hidden_size(); }
};
}