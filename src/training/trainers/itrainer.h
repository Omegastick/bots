#pragma once

#include <memory>
#include <string>
#include <vector>

#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
class ITrainer
{
  public:
    virtual ~ITrainer() = 0;

    virtual void save_model() = 0;
    virtual void step() = 0;
    virtual void slow_step() = 0;

    std::vector<std::unique_ptr<IEnvironment>> environments;
};

inline ITrainer::~ITrainer() {}
}
