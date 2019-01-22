#pragma once

#include "communicator.h"
#include "training/trainers/itrainer.h"

namespace SingularityTrainer
{
class QuickTrainer : ITrainer
{
  public:
    QuickTrainer(Communicator &communicator);
    ~QuickTrainer();

    virtual void begin_training();
    virtual void end_training();
    virtual void save_model();
    virtual void step();

    std::vector<std::unique_ptr<IEnvironment>> environments;

  private:
    Communicator &communicator;
};
}