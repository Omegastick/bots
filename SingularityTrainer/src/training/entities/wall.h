#pragma once

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class Wall : public IDrawable
{
  public:
    Wall(float x, float y, float width, float height, b2World &world);
    ~Wall();

    virtual RenderData get_render_data(bool lightweight = false);

  private:
    std::unique_ptr<RigidBody> rigid_body;
};
}