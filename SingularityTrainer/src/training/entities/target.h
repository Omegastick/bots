#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"
#include "training/icollidable.h"

namespace SingularityTrainer
{
class IEnvironment;
class Sprite;
class RigidBody;

class Target : public IDrawable, public ICollidable
{
  private:
    IEnvironment &environment;
    std::unique_ptr<Sprite> sprite;

  public:
    Target(float x, float y, b2World &world, IEnvironment &env);
    ~Target();

    RenderData get_render_data(bool lightweight = false);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);

    std::unique_ptr<RigidBody> rigid_body;
};
}