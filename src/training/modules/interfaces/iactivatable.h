#pragma once

namespace SingularityTrainer
{
class IActivatable
{
  public:
    IActivatable(){};
    ~IActivatable(){};

    virtual void activate() = 0;
};
}