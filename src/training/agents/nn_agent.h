#pragma once

#include <cpprl/model/policy.h>
#include <nlohmann/json_fwd.hpp>

#include "iagent.h"

namespace SingularityTrainer
{
class NNAgent : public IAgent
{
  private:
    mutable cpprl::Policy policy;

  public:
    NNAgent(cpprl::Policy policy, const nlohmann::json &body_spec, const std::string &name);

    ActResult act(torch::Tensor observations,
                  torch::Tensor hidden_states,
                  torch::Tensor masks) const;
    virtual std::unique_ptr<IAgent> clone() const;

    inline int get_hidden_state_size() const { return policy->get_hidden_size(); }
    inline void set_policy(cpprl::Policy policy) { this->policy = policy; }
};
}