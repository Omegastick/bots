#pragma once

namespace SingularityTrainer
{
class IShootable
{
  public:
    IShootable(){};
    ~IShootable(){};

    virtual void shoot() = 0;
};
}