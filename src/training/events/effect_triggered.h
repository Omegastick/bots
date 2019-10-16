#pragma once

#include <tuple>

#include <msgpack.hpp>

#include "misc/transform.h"
#include "training/events/ievent.h"

namespace SingularityTrainer
{
class IEnvironment;
class Random;

enum class EffectTypes
{
    BulletExplosion = 0,
    ThrusterParticles = 1
};

class EffectTriggered : public IEvent
{
  private:
    EffectTypes effect_type;
    double time;
    Transform transform;

  public:
    EffectTriggered(EffectTypes effect_type, double time, Transform transform);

    void trigger(IEnvironment &env);

    inline double get_time() const { return time; }
    inline Transform get_transform() const { return transform; }
    inline EffectTypes get_effect_type() const { return effect_type; }
};
}

MSGPACK_ADD_ENUM(SingularityTrainer::EffectTypes)