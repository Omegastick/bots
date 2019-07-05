#pragma once

#include <memory>
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
                          torch::Tensor masks) const = 0;
    virtual std::unique_ptr<IAgent> clone() const = 0;
    int get_action_size() const;
    const nlohmann::json &get_body_spec() const;
    virtual int get_hidden_state_size() const = 0;
    int get_observation_size() const;
};

inline IAgent::~IAgent() {}
}