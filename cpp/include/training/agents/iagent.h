#pragma once

#include <memory>
#include <vector>

#include "idrawable.h"
#include "training/actions/iaction.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class IAgent : IDrawable
{
  public:
    virtual void act(std::vector<int> actions) = 0;
    virtual std::vector<float> get_observation() = 0;

    std::vector<std::unique_ptr<IModule>> modules;
    std::vector<IAction *> actions;
};
}