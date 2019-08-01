#include <Box2D/Box2D.h>

#include "ientity.h"

namespace SingularityTrainer
{
IEntity::IEntity(unsigned int id) : id(id) {}

void IEntity::destroy()
{
    rigid_body->body->GetWorld()->DestroyBody(rigid_body->body);
}

void IEntity::set_transform(b2Vec2 position, float velocity)
{
    rigid_body->body->SetTransform(position, velocity);
}

void IEntity::set_angular_velocity(float angular_velocity)
{
    rigid_body->body->SetAngularVelocity(angular_velocity);
}

void IEntity::set_linear_velocity(b2Vec2 linear_velocity)
{
    rigid_body->body->SetLinearVelocity(linear_velocity);
}
}