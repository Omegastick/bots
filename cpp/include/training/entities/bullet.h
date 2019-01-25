#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class Bullet : public IDrawable, public ICollidable
{
  public:
    Bullet(b2Vec2 position, b2Vec2 velocity, b2World &world);
    ~Bullet();

    virtual void draw(sf::RenderTarget &render_target);
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);

  private:
    sf::CircleShape shape;
    std::unique_ptr<RigidBody> rigid_body;
};
}