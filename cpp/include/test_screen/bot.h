#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
class Bot : IDrawable
{
  public:
    Bot(ResourceManager &resource_manager);
    ~Bot();

    void act(std::vector<bool> actions);
    void draw(sf::RenderTarget &render_target);

  private:
    b2BodyDef body_def;
    b2PolygonShape polygon_shape;
    b2FixtureDef fixture_def;
    sf::Sprite sprite;
};
}