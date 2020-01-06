#pragma once

namespace ai
{
class RigidBody;

class ICollidable
{
  public:
    virtual void begin_contact(RigidBody *other) = 0;
    virtual void end_contact(RigidBody *other) = 0;
};
}