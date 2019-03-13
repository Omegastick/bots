#pragma once

#include <vector>

#include <Box2D/Box2D.h>

#include "training/agents/iagent.h"

namespace SingularityTrainer
{
class RenderData;
class Random;
class IEnvironment;

class TestAgent : public IAgent
{
  private:
    IEnvironment *environment;

    void register_actions();

  public:
    TestAgent(b2World &world, Random *rng, IEnvironment &environment);
    ~TestAgent();

    virtual void act(std::vector<int> action_flags);
    virtual std::vector<float> get_observation();
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    virtual RenderData get_render_data(bool lightweight = false);
    virtual void update_body();

    virtual inline IEnvironment *get_environment() const { return environment; }
};
}