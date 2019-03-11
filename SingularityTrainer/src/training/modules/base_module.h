#pragma once

#include <vector>

#include <Box2D/Box2D.h>

#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class IAgent;

class BaseModule : public IModule
{
  public:
    BaseModule(b2Body &body, IAgent *agent);
    ~BaseModule();

    virtual std::vector<float> get_sensor_reading();
};
}