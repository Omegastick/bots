#pragma once

#include <tuple>

#include <nlohmann/json.hpp>
#include <torch/torch.h>

namespace SingularityTrainer
{
// Actions, Hiden states
typedef std::tuple<torch::Tensor, torch::Tensor> ActResult;

class IAgent
{
  protected:
    nlohmann::json body_spec;

    IAgent(const nlohmann::json &body_spec);

  public:
    virtual ~IAgent() = 0;

    virtual ActResult act(torch::Tensor observations,
                          torch::Tensor hidden_states,
                          torch::Tensor masks) = 0;
    int get_action_size();
    const nlohmann::json &get_body_spec();
    virtual int get_hidden_state_size() = 0;
    int get_observation_size();
};

inline IAgent::~IAgent() {}
}