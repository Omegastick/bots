#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include <doctest/trompeloeil.hpp>

#include "training/agents/iagent.h"
#include "training/environments/ienvironment.h"

namespace cpprl
{
class Policy;
class Random;
class RolloutStorage;
}

namespace SingularityTrainer
{
class Renderer;

class ISingleRolloutGenerator
{
  public:
    virtual ~ISingleRolloutGenerator() = 0;

    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
    virtual cpprl::RolloutStorage generate(unsigned long length) = 0;
};

inline ISingleRolloutGenerator::~ISingleRolloutGenerator() {}

class SingleRolloutGenerator : public ISingleRolloutGenerator
{
  private:
    const IAgent &agent;
    std::unique_ptr<IEnvironment> environment;
    torch::Tensor hidden_state;
    torch::Tensor last_observation;
    std::mutex mutex;
    const IAgent *opponent;
    torch::Tensor opponent_hidden_state;
    torch::Tensor opponent_last_observation;
    torch::Tensor opponent_mask;
    const std::vector<std::unique_ptr<IAgent>> &opponent_pool;
    bool reset_recently;
    Random &rng;
    float score;
    bool slow;
    bool start_position;

  public:
    SingleRolloutGenerator(const IAgent &agent,
                           std::unique_ptr<IEnvironment> environment,
                           const std::vector<std::unique_ptr<IAgent>> &opponent_pool,
                           Random &rng);

    void draw(Renderer &renderer, bool lightweight = false) override;
    cpprl::RolloutStorage generate(unsigned long length) override;
};

class MockSingleRolloutGenerator : public trompeloeil::mock_interface<ISingleRolloutGenerator>
{
  public:
    IMPLEMENT_MOCK2(draw);
    IMPLEMENT_MOCK1(generate);
};
}