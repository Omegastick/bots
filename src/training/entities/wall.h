#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"
#include "training/icollidable.h"

namespace SingularityTrainer
{
class Sprite;
class RigidBody;

class Wall : public IDrawable, public ICollidable
{
  private:
    std::unique_ptr<RigidBody> rigid_body;
    std::unique_ptr<Sprite> sprite;

  public:
    Wall(float x, float y, float width, float height, b2World &world);
    ~Wall();

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
};
}