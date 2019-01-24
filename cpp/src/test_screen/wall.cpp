#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "gui/colors.h"
#include "test_screen/wall.h"

namespace SingularityTrainer
{
Wall::Wall(float x, float y, float width, float height, b2World &world)
{
    // Shape
    shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(x, y);
    shape.setFillColor(cl_white);

    // Rigidbody
    b2Vec2 position(x + (width / 2), y + (height / 2));
    rigid_body = std::make_unique<RigidBody>(b2_staticBody, position, world, this, RigidBody::ParentTypes::Wall);
    b2PolygonShape rigid_body_shape;
    rigid_body_shape.SetAsBox(width / 2, height / 2);
    b2FixtureDef fixture_def;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.shape = &rigid_body_shape;
    rigid_body->body->CreateFixture(&fixture_def);
}

Wall::~Wall() {}

void Wall::draw(sf::RenderTarget &render_target)
{
    render_target.draw(shape);
}
}