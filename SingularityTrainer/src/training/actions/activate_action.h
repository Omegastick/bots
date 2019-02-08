#pragma once

#include "training/actions/iaction.h"
#include "training/modules/interfaces/iactivatable.h"

namespace SingularityTrainer
{
class ActivateAction : public IAction
{
  public:
    ActivateAction(IActivatable *module);
    ~ActivateAction();

    virtual void act(std::vector<int> flags);

    IActivatable *module;
};
}