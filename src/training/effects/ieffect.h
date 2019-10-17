#pragma once

namespace SingularityTrainer
{
class Renderer;

class IEffect
{
  public:
    virtual ~IEffect() = 0;

    virtual void trigger(Renderer &renderer) = 0;
};

inline IEffect::~IEffect() {}
}