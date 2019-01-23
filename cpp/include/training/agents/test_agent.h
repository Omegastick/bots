#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "training/actions/iaction.h"
#include "training/rigid_body.h"
#include "training/agents/iagent.h"

namespace SingularityTrainer
{
class TestAgent : public IAgent
{
  public:
    TestAgent();
    ~TestAgent();

    virtual void act(std::vector<int> actions);
    virtual std::vector<float> get_observation();
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    virtual void draw(sf::RenderTarget &render_target);
};
}