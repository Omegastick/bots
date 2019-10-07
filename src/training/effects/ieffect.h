#pragma once

namespace SingularityTrainer
{
struct RenderData;

class IEffect
{
  public:
    virtual ~IEffect() = 0;

    virtual RenderData trigger() = 0;
};

inline IEffect::~IEffect() {}
}