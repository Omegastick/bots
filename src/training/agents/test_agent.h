#pragma once

#include <vector>

#include <Box2D/Box2D.h>

#include "training/agents/agent.h"

namespace SingularityTrainer
{
class RenderData;
class Random;
class IEnvironment;

class TestAgent : public Agent
{
  public:
    TestAgent(b2World &world, Random *rng, IEnvironment &environment);
};
}