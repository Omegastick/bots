#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>

#include "idrawable.h"
#include "test_screen/rigid_body.h"

namespace SingularityTrainer
{
class Target : public IDrawable
{
  public:
    Target(float x, float y, b2World &world);
    ~Target();

    void draw(sf::RenderTarget &render_target);

  private:
    std::unique_ptr<RigidBody> rigid_body;
    sf::CircleShape shape;
};
}