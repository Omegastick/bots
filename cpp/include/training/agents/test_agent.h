#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>

#include "idrawable.h"
#include "training/actions/iaction.h"
#include "training/rigid_body.h"
#include "training/agents/iagent.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
class TestAgent : public IAgent
{
  public:
    TestAgent(ResourceManager &resource_manager, b2World &world);
    ~TestAgent();

    virtual void act(std::vector<int> actions);
    virtual std::vector<float> get_observation();
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    virtual void draw(sf::RenderTarget &render_target);
    virtual void update_body();
};
}