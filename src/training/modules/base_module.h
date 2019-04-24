#pragma once

#include <vector>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

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
    virtual nlohmann::json to_json() const;
};
}