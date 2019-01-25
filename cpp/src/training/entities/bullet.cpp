#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "training/entities/bullet.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
Bullet::Bullet(b2Vec2 position, b2Vec2 velocity, b2World &world) : shape(0.1)
{
    shape.setOrigin(0.1, 0.1);

    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, position, world, this, RigidBody::ParentTypes::Bullet);

    b2CircleShape rigid_body_shape;
    rigid_body_shape.m_radius = 0.1;
    b2FixtureDef fixture_def;
    fixture_def.shape = &rigid_body_shape;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.isSensor = true;
    rigid_body->body->CreateFixture(&fixture_def);

    rigid_body->body->SetBullet(true);
    rigid_body->body->ApplyForceToCenter(velocity, true);
}

Bullet::~Bullet() {}

void Bullet::draw(sf::RenderTarget &render_target)
{
    b2Vec2 position = rigid_body->body->GetPosition();
    shape.setPosition(position.x, position.y);
    render_target.draw(shape);
}

void Bullet::begin_contact(RigidBody *other) {}
void Bullet::end_contact(RigidBody *other) {}
}