#pragma once

#include "training/actions/iaction.h"

namespace ai
{
class IActivatable;

class ActivateAction : public IAction
{
  public:
    ActivateAction(IActivatable *module);
    ~ActivateAction();

    virtual void act(std::vector<int> flags);

    IActivatable *module;
};
}