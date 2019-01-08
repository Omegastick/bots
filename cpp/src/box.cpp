#include <random>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <iostream>

#include "box.h"

Box::Box(const b2Vec2 &size, const b2Vec2 &position, b2World &world)
{
    // Shape
    sf::Color color(std::rand() % 255, std::rand() % 255, std::rand() % 255, 255);
    shape = sf::RectangleShape(sf::Vector2f(size.x * 20, size.y * 20));
    shape.setOrigin(size.x * 10, size.y * 10);
    shape.setFillColor(color);

    // Rigidbody
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(position.x, position.y);
    body = world.CreateBody(&bodyDef);
    polygonShape.SetAsBox(size.x, size.y);
    fixtureDef.shape = &polygonShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 1.0f;
    body->CreateFixture(&fixtureDef);
}

Box::~Box() {}