#pragma once

#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>

class Box
{
  public:
    sf::RectangleShape shape;
    b2Body* body;

    Box(const b2Vec2 &size, const b2Vec2 &position, b2World &world);
    ~Box();

  private:
    sf::Image image;
    sf::Texture texture;
    b2BodyDef bodyDef;
    b2PolygonShape polygonShape;
    b2FixtureDef fixtureDef;
};