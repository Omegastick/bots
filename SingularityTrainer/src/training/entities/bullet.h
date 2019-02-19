#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "graphics/idrawable.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class Bullet : public IDrawable, public ICollidable
{
  public:
    Bullet(b2Vec2 position, b2Vec2 velocity, b2World &world);
    ~Bullet();

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    void update();

    bool destroyed;
    int life;
    std::unique_ptr<RigidBody> rigid_body;

  private:
    b2Vec2 last_position;
};
}