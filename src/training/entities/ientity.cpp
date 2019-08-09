#include <Box2D/Box2D.h>

#include "ientity.h"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
IEntity::IEntity(unsigned int id, IEnvironment &env) : id(id), env(env) {}

void IEntity::destroy()
{
    rigid_body->body->GetWorld()->DestroyBody(rigid_body->body);

    env.get_entities().erase(id);
}

b2Transform IEntity::get_transform() const
{
    return rigid_body->body->GetTransform();
}

float IEntity::get_angular_velocity() const
{
    return rigid_body->body->GetAngularVelocity();
}

b2Vec2 IEntity::get_linear_velocity() const
{
    return rigid_body->body->GetLinearVelocity();
}

void IEntity::set_transform(b2Vec2 position, float rotation)
{
    rigid_body->body->SetTransform(position, rotation);
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