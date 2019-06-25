#pragma once

#include <vector>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class Body;

class BaseModule : public IModule
{
  public:
    BaseModule();
    ~BaseModule();

    virtual std::vector<float> get_sensor_reading() const;
    virtual nlohmann::json to_json() const;

    inline int get_observation_count() const { return 3; }
};
}