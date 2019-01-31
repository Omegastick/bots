#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <iostream>

#include "gui/colors.h"
#include "idrawable.h"
#include "training/entities/bullet.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
Bullet::Bullet(b2Vec2 position, b2Vec2 velocity, b2World &world)
    : shape(0.1), destroyed(false), life(10), trail(sf::Triangles, 3), last_position(b2Vec2_zero)
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

    // Trail colour
    sf::Color trail_end_colour = cl_white;
    trail_end_colour.a = 0;
    trail[0].color = trail_end_colour;
    trail[1].color = cl_white;
    trail[2].color = cl_white;

    rigid_body->body->ApplyForceToCenter(velocity, true);
}

Bullet::~Bullet() {}

void Bullet::draw(sf::RenderTarget &render_target, bool lightweight)
{
    if (!destroyed)
    {
        b2Vec2 position = rigid_body->body->GetPosition();
        b2Vec2 velocity = rigid_body->body->GetLinearVelocity();
        velocity.Normalize();
        velocity *= 0.1;

        // Body
        shape.setPosition(position.x, position.y);
        render_target.draw(shape);

        // Trail
        if (last_position.x != b2Vec2_zero.x || last_position.y != b2Vec2_zero.y)
        {
            trail[0].position = sf::Vector2f(last_position.x, last_position.y);
            trail[1].position = sf::Vector2f(position.x + velocity.y, position.y - velocity.x);
            trail[2].position = sf::Vector2f(position.x - velocity.y, position.y + velocity.x);
            render_target.draw(trail);
        }

        last_position = position;
    }
}

void Bullet::begin_contact(RigidBody *other)
{
    destroyed = true;
}

void Bullet::end_contact(RigidBody *other) {}

void Bullet::update()
{
    destroyed = --life <= 0;
}
}