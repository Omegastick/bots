#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "idrawable.h"
#include "test_screen/bot.h"
#include "test_screen/icollidable.h"
#include "training/rigid_body.h"
#include "test_screen/test_env.h"

namespace SingularityTrainer
{
class TestEnv;

class Target : public IDrawable, public ICollidable
{
  public:
    Target(float x, float y, b2World &world, TestEnv &env);
    ~Target();

    void draw(sf::RenderTarget &render_target);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);

  private:
    std::unique_ptr<RigidBody> rigid_body;
    sf::CircleShape shape;
    TestEnv &environment;
};
}