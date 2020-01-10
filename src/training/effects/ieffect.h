#pragma once

namespace ai
{
class IAudioEngine;
class Renderer;

class IEffect
{
  public:
    virtual ~IEffect() = 0;

    virtual void trigger(Renderer &renderer, IAudioEngine *audio_engine = nullptr) = 0;
};

inline IEffect::~IEffect() {}
}