#pragma once

#include <memory>
#include <string>
#include <tuple>

#include <nlohmann/json.hpp>
#include <torch/torch.h>

namespace SingularityTrainer
{
struct ActResult
{
    torch::Tensor value, action, log_probs, hidden_state;
};

class IAgent
{
  protected:
    nlohmann::json body_spec;
    std::string name;

    IAgent(const nlohmann::json &body_spec, const std::string &name);

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

    inline std::string get_name() const { return name; }
    inline void set_name(const std::string &name) { this->name = name; }
};

inline IAgent::~IAgent() {}
}