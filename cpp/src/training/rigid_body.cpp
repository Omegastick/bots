#include <Box2D/Box2D.h>

#include "training/rigid_body.h"

namespace SingularityTrainer
{
RigidBody::RigidBody(b2BodyType type, b2Vec2 position, b2World &world, b2Shape &shape, void *parent, RigidBody::ParentTypes parent_type)
    : parent(parent),
      parent_type(parent_type)
{
    // Rigidbody
    body_def.type = type;
    body_def.position = position;
    body = world.CreateBody(&body_def);
    body->SetUserData(this);
    fixture_def.shape = &shape;
    fixture_def.density = 1.0;
    fixture_def.friction = 1.0;
    body->CreateFixture(&fixture_def);
}

RigidBody::~RigidBody() {}
}
