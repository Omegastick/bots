#pragma once

#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class SquareHull : public IModule
{
  public:
    SquareHull();

    virtual nlohmann::json to_json() const;

    inline int get_observation_count() const { return 0; }
};
}