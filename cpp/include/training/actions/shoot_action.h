#pragma once

#include "training/actions/iaction.h"
#include "training/modules/interfaces/ishootable.h"

namespace SingularityTrainer
{
class ShootAction : public IAction
{
  public:
    ShootAction(IShootable *module);
    ~ShootAction();

    virtual void act(std::vector<int> flags);

    IShootable *module;
};
}