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

    std::vector<int> act(torch::Tensor observation,
                         torch::Tensor hidden_state,
                         torch::Tensor mask);
    std::vector<std::vector<int>> act_multiple(torch::Tensor observations,
                                               torch::Tensor hidden_states,
                                               torch::Tensor masks);

    inline int get_hidden_state_size() { return 0; }
};
}