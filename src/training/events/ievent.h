#pragma once

#include <msgpack.hpp>

namespace SingularityTrainer
{
class IEnvironment;

enum class EventTypes
{
    EntityDestroyed = 0
};

class IEvent
{
  public:
    EventTypes type;

    virtual ~IEvent() = 0;

    virtual double get_time() const = 0;
    virtual void trigger(IEnvironment &env) = 0;

    MSGPACK_DEFINE_ARRAY(type)
};

inline IEvent::~IEvent() {}
}

MSGPACK_ADD_ENUM(SingularityTrainer::EventTypes)
