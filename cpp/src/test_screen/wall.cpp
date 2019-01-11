#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "test_screen/wall.h"

namespace SingularityTrainer
{
Wall::Wall(float x, float y, float width, float height, b2World &world)
{
    // Shape
    shape.setSize(sf::Vector2f(width * 100, height * 100));
    shape.setPosition(x * 100, y * 100);

    // Rigidbody
    body_def.type = b2_staticBody;
    body_def.position.Set(x + (width / 2), y + (height / 2));
    body = world.CreateBody(&body_def);
    polygon_shape.SetAsBox(width / 2, height / 2);
    fixture_def.shape = &polygon_shape;
    fixture_def.density = 1.0f;
    fixture_def.friction = 1.0f;
    body->CreateFixture(&fixture_def);

    // Labels
    labels = std::vector<std::string>{"wall"};
}

Wall::~Wall() {}

Wall::Wall(Wall &&other) = default;

void Wall::draw(sf::RenderTarget &render_target)
{
    render_target.draw(shape);
}
}