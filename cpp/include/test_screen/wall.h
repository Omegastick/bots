#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class Wall : public IDrawable
{
  public:
    Wall(float x, float y, float width, float height, b2World &world);
    ~Wall();

    void draw(sf::RenderTarget &render_target);

  private:
    std::unique_ptr<RigidBody> rigid_body;
    sf::RectangleShape shape;
};
}