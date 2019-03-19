#pragma once

#include <vector>

#include <Box2D/Box2D.h>

#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class Agent;

class BaseModule : public IModule
{
  public:
    BaseModule();
    ~BaseModule();

    virtual std::vector<float> get_sensor_reading();
};
}