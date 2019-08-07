#pragma once

#include <memory>
#include <vector>

#include <torch/torch.h>

#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
class IAgent;

class ClientAgent
{
  private:
    std::unique_ptr<IAgent> agent;
    int agent_number;
    std::unique_ptr<IEnvironment> env;
    torch::Tensor hidden_state;

  public:
    ClientAgent(std::unique_ptr<IAgent> agent,
                int agent_number,
                std::unique_ptr<IEnvironment> env);

    std::vector<int> get_action(const EnvState &env_state);
    void set_bodies(const std::vector<nlohmann::json> &body_specs);
};
}