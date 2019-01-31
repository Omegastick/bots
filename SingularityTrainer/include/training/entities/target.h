#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>

#include "idrawable.h"
#include "training/environments/ienvironment.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class Target : public IDrawable, public ICollidable
{
  public:
    Target(float x, float y, b2World &world, IEnvironment &env);
    ~Target();

    void draw(sf::RenderTarget &render_target, bool lightweight = false);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);

  private:
    std::unique_ptr<RigidBody> rigid_body;
    sf::CircleShape shape;
    IEnvironment &environment;
};
}