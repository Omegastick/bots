#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>

#include "gui/colors.h"
#include "idrawable.h"
#include "training/entities/target.h"
#include "training/environments/ienvironment.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
Target::Target(float x, float y, b2World &world, IEnvironment &env) : environment(env)
{
    // Rigid body
    rigid_body = std::make_unique<RigidBody>(b2_staticBody, b2Vec2(x, y), world, this, RigidBody::ParentTypes::Target);
    b2CircleShape rigid_body_shape;
    rigid_body_shape.m_radius = 0.5;
    b2FixtureDef fixture_def;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.shape = &rigid_body_shape;
    rigid_body->body->CreateFixture(&fixture_def);

    // Sprite
    shape.setFillColor(cl_white);
    shape.setRadius(0.5);
    shape.setOrigin(0.5, 0.5);
    shape.setPosition(x, y);
}

Target::~Target() {}

void Target::draw(sf::RenderTarget &render_target, bool lightweight)
{
    render_target.draw(shape);
}

void Target::begin_contact(RigidBody *other)
{
    if (other->parent_type == RigidBody::ParentTypes::Bullet)
    {
        environment.change_reward(1);
    }
}

void Target::end_contact(RigidBody *other)
{
}
}