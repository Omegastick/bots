#pragma once

namespace SingularityTrainer
{
class RigidBody;

class ICollidable
{
    virtual void begin_contact(RigidBody *other) = 0;
    virtual void end_contact(RigidBody *other) = 0;
};
}