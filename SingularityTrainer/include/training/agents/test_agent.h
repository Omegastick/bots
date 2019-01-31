#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <vector>

#include "idrawable.h"
#include "resource_manager.h"
#include "training/actions/iaction.h"
#include "training/agents/iagent.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class TestAgent : public IAgent
{
  public:
    TestAgent(ResourceManager &resource_manager, b2World &world);
    ~TestAgent();

    virtual void act(std::vector<int> action_flags);
    virtual std::vector<float> get_observation();
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    virtual void draw(sf::RenderTarget &render_target);
    virtual void update_body();

  private:
    void register_actions();
};
}