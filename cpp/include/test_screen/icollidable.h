#pragma once

#include "test_screen/rigid_body.h"

namespace SingularityTrainer
{
class ICollidable {
    virtual void begin_contact(RigidBody* other) = 0;
    virtual void end_contact(RigidBody* other) = 0;
};
}