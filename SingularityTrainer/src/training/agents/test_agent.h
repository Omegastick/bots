#pragma once

#include <vector>

#include <Box2D/Box2D.h>

#include "training/agents/iagent.h"

namespace SingularityTrainer
{
class RenderData;
class Random;

class TestAgent : public IAgent
{
  public:
    TestAgent(b2World &world, Random *rng);
    ~TestAgent();

    virtual void act(std::vector<int> action_flags);
    virtual std::vector<float> get_observation();
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    virtual RenderData get_render_data(bool lightweight = false);
    virtual void update_body();

  private:
    void register_actions();
};
}