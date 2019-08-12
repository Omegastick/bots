#pragma once

#include <tuple>

#include "training/events/ievent.h"

namespace SingularityTrainer
{
class IEnvironment;
class Random;

typedef std::tuple<float, float, float> Transform;

enum class EffectTypes
{
    BulletExplosion = 0,
    ThrusterParticles = 1
};

class EffectTriggered : public IEvent
{
  private:
    EffectTypes effect_type;
    Random &rng;
    double time;
    Transform transform;

  public:
    EffectTriggered(EffectTypes effect_type, double time, Transform transform, Random &rng);

    void trigger(IEnvironment &env);

    inline double get_time() const { return time; }
    inline Transform get_transform() const { return transform; }
    inline EffectTypes get_effect_type() const { return effect_type; }
};
}