#pragma once

#include <memory>
#include <vector>

#include "training/rollout_generators/single_rollout_generator.h"

namespace SingularityTrainer
{
class Renderer;

class MultiRolloutGenerator
{
  private:
    unsigned long batch_number;
    unsigned long num_steps;
    std::vector<std::unique_ptr<ISingleRolloutGenerator>> sub_generators;

  public:
    MultiRolloutGenerator(unsigned long num_steps,
                          std::vector<std::unique_ptr<ISingleRolloutGenerator>> &&sub_generators);

    void draw(Renderer &renderer, bool lightweight = false);
    cpprl::RolloutStorage generate();

    inline unsigned long get_batch_number() const { return batch_number; }
};
}