#pragma once

#include "iagent.h"

namespace SingularityTrainer
{
class Random;

class RandomAgent : public IAgent
{
  private:
    int num_outputs;
    Random &rng;

  public:
    RandomAgent(int num_outputs, Random &rng);

    ActResult act(torch::Tensor observations,
                  torch::Tensor hidden_states,
                  torch::Tensor masks);

    inline int get_hidden_state_size() { return 0; }
};
}