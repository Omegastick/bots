#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "idrawable.h"

namespace SingularityTrainer
{
class Wall : IDrawable
{
  public:
    Wall(float x, float y, float width, float height, b2World &world);
    ~Wall();
    Wall(Wall&& other);

    void draw(sf::RenderTarget &render_target);

  private:
    b2Body *body;
    b2BodyDef body_def;
    b2PolygonShape polygon_shape;
    b2FixtureDef fixture_def;
    sf::RectangleShape shape;
};
}