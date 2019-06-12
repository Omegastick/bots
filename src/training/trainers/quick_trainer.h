#pragma once

#include <chrono>
#include <vector>

#include <cpprl/cpprl.h>
#include <torch/torch.h>

#include "third_party/di.hpp"
#include "training/environments/ienvironment.h"
#include "training/score_processor.h"
#include "training/trainers/itrainer.h"

namespace SingularityTrainer
{
class Agent;
class AgentFactory;
class IEnvironmentFactory;
class Random;

auto EnvCount = [] {};

class QuickTrainer : public ITrainer
{
  private:
    int agents_per_env;
    std::unique_ptr<cpprl::Algorithm> algorithm;
    cpprl::Policy policy;
    std::shared_ptr<cpprl::NNBase> nn_base;
    cpprl::RolloutStorage rollout_storage;
    bool waiting;
    torch::Tensor observations;
    int env_count;
    int frame_counter;
    int action_frame_counter;
    float elapsed_time;
    std::unique_ptr<ScoreProcessor> score_processor;
    std::vector<float> env_scores;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;

    void action_update();

  public:
    BOOST_DI_INJECT(QuickTrainer,
                    (named = EnvCount) int env_count,
                    IEnvironmentFactory &env_factory,
                    AgentFactory &agent_factory);

    virtual void save_model();
    virtual void step();
    virtual void slow_step();
};
}