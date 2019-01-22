#pragma once

#include <vector>

#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class IAction
{
  public:
    IModule *module;
    int flag_count;

    virtual void act(std::vector<int> flags) = 0;
};
}