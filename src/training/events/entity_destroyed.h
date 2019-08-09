#pragma once

#include <tuple>

#include "training/events/ievent.h"

namespace SingularityTrainer
{
class IEnvironment;

typedef std::tuple<float, float, float> Transform;

class EntityDestroyed : public IEvent
{
  private:
    unsigned int entity_id;
    double time;
    Transform transform;

  public:
    EntityDestroyed(unsigned int entity_id, double time, Transform transform);

    void trigger(IEnvironment &env);

    inline double get_time() const { return time; }
};
}