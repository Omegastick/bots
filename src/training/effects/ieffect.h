#pragma once

namespace SingularityTrainer
{
class RenderData;

class IEffect
{
  public:
    virtual ~IEffect() = 0;

    virtual RenderData trigger() = 0;
};

inline IEffect::~IEffect() {}
}