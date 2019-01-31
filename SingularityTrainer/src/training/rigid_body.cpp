#include <Box2D/Box2D.h>

#include "training/rigid_body.h"

namespace SingularityTrainer
{
RigidBody::RigidBody(b2BodyType type, b2Vec2 position, b2World &world, void *parent, RigidBody::ParentTypes parent_type)
    : parent(parent),
      parent_type(parent_type)
{
    // Rigidbody
    body_def.type = type;
    body_def.position = position;
    bool x = world.IsLocked();
    body = world.CreateBody(&body_def);
    body->SetUserData(this);
}

RigidBody::~RigidBody() {}
}
