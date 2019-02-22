#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "graphics/sprite.h"
#include "graphics/idrawable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class Wall : public IDrawable
{
  private:
    std::unique_ptr<RigidBody> rigid_body;
    std::unique_ptr<Sprite> sprite;

  public:
    Wall(float x, float y, float width, float height, b2World &world);
    ~Wall();

    virtual RenderData get_render_data(bool lightweight = false);
};
}