#pragma once

#include <memory>
#include <vector>

#include "communicator.h"
#include "resource_manager.h"
#include "training/trainers/itrainer.h"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
class QuickTrainer : public ITrainer
{
  public:
    QuickTrainer(ResourceManager &resource_manager, Communicator *communicator, int env_count);
    ~QuickTrainer();

    virtual void begin_training();
    virtual void end_training();
    virtual void save_model();
    virtual void step();
    virtual void slow_step();

  private:
    Communicator *communicator;
    bool waiting_for_server;
    std::vector<std::vector<float>> observations;
    int env_count;
    int frame_counter;
    int action_frame_counter;

    void action_update();
};
}