#include <Box2D/Box2D.h>

#include "training/rigid_body.h"

namespace SingularityTrainer
{
RigidBody::RigidBody(b2BodyType type, b2Vec2 position, b2World &world, void *parent, RigidBody::ParentTypes parent_type)
    : parent(parent),
      parent_type(parent_type),
      world(&world)
{
    body_def.type = type;
    body_def.position = position;
    create_body();
}

RigidBody::~RigidBody() {}

void RigidBody::create_body()
{
    body = world->CreateBody(&body_def);
    body->SetUserData(this);
}

void RigidBody::destroy()
{
    world->DestroyBody(body);
    body = nullptr;
}
}
