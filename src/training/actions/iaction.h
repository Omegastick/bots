#pragma once

#include <vector>

namespace ai
{
class IAction
{
  public:
    virtual ~IAction() = 0;

    virtual void act(std::vector<int> flags) = 0;

    int flag_count;
};

inline IAction::~IAction() {}
}