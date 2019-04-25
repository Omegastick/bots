#pragma once

#include <vector>

#include <cpprl/model/policy.h>
#include <cpprl/storage.h>
#include <torch/torch.h>

#include "training/trainers/itrainer.h"

namespace cpprl
{
class Algorithm;
class NNBase;
}

namespace SingularityTrainer
{
class Random;
class ScoreProcessor;

class QuickTrainer : public ITrainer
{
  private:
    std::unique_ptr<cpprl::Algorithm> algorithm;
    cpprl::Policy policy;
    std::shared_ptr<cpprl::NNBase> nn_base;
    cpprl::RolloutStorage rollout_storage;

    bool waiting;
    torch::Tensor observations;
    int env_count;
    int frame_counter;
    int action_frame_counter;
    Random *rng;
    float elapsed_time;
    std::unique_ptr<ScoreProcessor> score_processor;
    std::vector<float> env_scores;
    int agents_per_env;

    void action_update();

  public:
    QuickTrainer(Random *rng, int env_count);
    ~QuickTrainer();

    virtual void begin_training();
    virtual void end_training();
    virtual void save_model();
    virtual void step();
    virtual void slow_step();
};
}