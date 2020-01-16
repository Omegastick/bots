#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "training/rollout_generators/single_rollout_generator.h"

namespace ai
{
class Renderer;

class MultiRolloutGenerator
{
  private:
    unsigned long batch_number;
    unsigned long num_steps;
    std::vector<std::unique_ptr<ISingleRolloutGenerator>> sub_generators;
    std::atomic<unsigned long long> timestep;

  public:
    MultiRolloutGenerator(unsigned long num_steps,
                          std::vector<std::unique_ptr<ISingleRolloutGenerator>> &&sub_generators);

    void draw(Renderer &renderer, bool lightweight = false);
    cpprl::RolloutStorage generate();
    void set_fast();
    void set_slow();
    void stop();

    inline unsigned long get_batch_number() const { return batch_number; }
    inline std::string get_current_opponent(int environment) const
    {
        return sub_generators[environment]->get_current_opponent();
    }
    std::vector<std::vector<float>> get_scores() const;

    inline unsigned long long get_timestep() { return timestep; }
};
}