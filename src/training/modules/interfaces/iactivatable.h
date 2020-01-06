#pragma once

namespace ai
{
class IActivatable
{
  public:
    IActivatable(){};
    ~IActivatable(){};

    virtual void activate() = 0;
};
}