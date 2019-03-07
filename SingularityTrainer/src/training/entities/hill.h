#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"
#include "training/environments/ienvironment.h"
#include "training/agents/iagent.h"
#include "graphics/sprite.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
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