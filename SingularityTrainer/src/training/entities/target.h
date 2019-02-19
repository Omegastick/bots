#pragma once

#include <Box2D/Box2D.h>
#include <memory>

#include "graphics/idrawable.h"
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

    RenderData get_render_data(bool lightweight = false);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);

    std::unique_ptr<RigidBody> rigid_body;

  private:
    IEnvironment &environment;
};
}