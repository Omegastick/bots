#pragma once

#include <memory>
#include <vector>

#include "communicator.h"
#include "random.h"
#include "resource_manager.h"
#include "training/environments/ienvironment.h"
#include "training/trainers/itrainer.h"
#include "training/score_processor.h"

namespace SingularityTrainer
{
class QuickTrainer : public ITrainer
{
  private:
    Communicator *communicator;
    bool waiting_for_server;
    std::vector<std::vector<std::vector<float>>> observations;
    int env_count;
    int frame_counter;
    int action_frame_counter;
    Random *rng;
    float elapsed_time;
    ScoreProcessor score_processor;
    std::vector<float> env_scores;
    int agents_per_env;

    void action_update();

  public:
    QuickTrainer(Communicator *communicator, Random *rng, int env_count);
    ~QuickTrainer();

    virtual void begin_training();
    virtual void end_training();
    virtual void save_model();
    virtual void step();
    virtual void slow_step();
};
}