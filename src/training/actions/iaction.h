#pragma once

#include <vector>

namespace SingularityTrainer
{
class IAction
{
  public:
    IAction(){};
    ~IAction(){};

    virtual void act(std::vector<int> flags) = 0;

    int flag_count;
};
}