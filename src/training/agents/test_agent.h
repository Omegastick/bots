#pragma once

#include <memory>
#include <vector>

#include "training/agents/agent.h"

namespace SingularityTrainer
{
class Random;
class RenderData;
class RigidBody;
class IEnvironment;

class TestAgent : public Agent
{
  public:
    TestAgent(Random &rng);
    TestAgent(std::unique_ptr<RigidBody> rigid_body, Random &rng, IEnvironment &environment);

    void setup();
};

class TestAgentFactory : public AgentFactory
{
  public:
    std::unique_ptr<Agent> make(Random &rng)
    {
        return std::make_unique<TestAgent>(rng);
    }

    virtual std::unique_ptr<Agent> make(b2World &world, Random &rng)
    {
        auto agent = std::make_unique<TestAgent>(rng);
        auto rigid_body = std::make_unique<RigidBody>(
            b2_dynamicBody,
            b2Vec2_zero,
            world,
            nullptr,
            RigidBody::ParentTypes::Agent);
        agent->set_rigid_body(std::move(rigid_body));
        agent->setup();
        return agent;
    }
};
}