#pragma once

#include <memory>
#include <unordered_map>

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
    std::unordered_map<IAgent *, int> occupants;
    std::unordered_map<IAgent *, int> registered_agents;

  public:
    Hill(float x, float y, b2World &world, IEnvironment &env);
    ~Hill();

    RenderData get_render_data(bool lightweight = false);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);
    void register_agent(IAgent *agent, int agent_number);
    void update() const;
    void reset();

    std::unique_ptr<RigidBody> rigid_body;
};
}