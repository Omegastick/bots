#include "test_screen/target.h"

namespace SingularityTrainer
{
Target::Target(float x, float y, b2World &world)
{
    // Rigid body
    b2CircleShape rigid_body_shape;
    rigid_body_shape.m_radius = 0.5;
    rigid_body = std::make_unique<RigidBody>(b2_staticBody, b2Vec2(x, y), world, rigid_body_shape, this, RigidBody::ParentTypes::Target);

    // Sprite
    shape.setFillColor(sf::Color::White);
    shape.setRadius(0.5);
    shape.setOrigin(0.5, 0.5);
    shape.setPosition(x, y);
}

Target::~Target(){};

void Target::draw(sf::RenderTarget &render_target)
{
    render_target.draw(shape);
}
}