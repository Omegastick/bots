#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"
#include "training/icollidable.h"

namespace SingularityTrainer
{
class IEnvironment;
class IAgent;
class Sprite;
class RigidBody;

class Hill : public IDrawable, public ICollidable
{
  private:
    IEnvironment &environment;
    std::unique_ptr<Sprite> sprite;
    std::vector<IAgent *> occupants;

  public:
    Hill(float x, float y, b2World &world, IEnvironment &env);
    ~Hill();

    RenderData get_render_data(bool lightweight = false);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);

    inline const std::vector<IAgent *> &get_occupants() const { return occupants; }

    std::unique_ptr<RigidBody> rigid_body;
};
}