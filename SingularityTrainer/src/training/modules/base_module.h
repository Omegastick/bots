#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include "resource_manager.h"
#include "training/actions/iaction.h"
#include "training/agents/iagent.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class BaseModule : public IModule
{
  public:
    BaseModule(b2Body &body, IAgent *agent);
    ~BaseModule();

    virtual std::vector<float> get_sensor_reading();
};
}