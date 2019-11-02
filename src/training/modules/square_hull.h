#pragma once

#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class SquareHull : public IModule
{
  public:
    SquareHull();

    virtual nlohmann::json to_json() const override final;

    inline virtual int get_observation_count() const override final { return 0; }
};
}