#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "training/icollidable.h"

namespace ai
{
struct Sprite;
class Renderer;
class RigidBody;

class Wall : public ICollidable
{
  private:
    std::unique_ptr<RigidBody> rigid_body;
    std::unique_ptr<Sprite> sprite;

  public:
    Wall(float x, float y, float width, float height, b2World &world);

    void draw(Renderer &renderer, bool lightweight = false);
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
};
}