#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include <trompeloeil.hpp>

#include "training/agents/iagent.h"
#include "environment/iecs_env.h"

namespace cpprl
{
class RolloutStorage;
}

namespace ai
{
class IAudioEngine;
class Random;
class Renderer;

class ISingleRolloutGenerator
{
  public:
    virtual ~ISingleRolloutGenerator() = 0;

    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
    virtual void fast_forward(unsigned int steps) = 0;
    virtual cpprl::RolloutStorage generate(unsigned long length) = 0;
    virtual std::string get_current_opponent() const = 0;
    virtual const IEcsEnv &get_environment() const = 0;
    virtual std::pair<float, float> get_scores() const = 0;
    virtual void set_fast() = 0;
    virtual void set_slow() = 0;
    virtual void set_timestep_pointer(std::atomic<unsigned long long> *timestep) = 0;
    virtual void set_audibility(bool visibility) = 0;
    virtual void stop() = 0;
};

inline ISingleRolloutGenerator::~ISingleRolloutGenerator() {}

class SingleRolloutGenerator : public ISingleRolloutGenerator
{
  private:
    const IAgent &agent;
    IAudioEngine &audio_engine;
    std::unique_ptr<IEcsEnv> environment;
    torch::Tensor hidden_state;
    torch::Tensor last_observation;
    mutable std::mutex mutex;
    const IAgent *opponent;
    torch::Tensor opponent_hidden_state;
    torch::Tensor opponent_last_observation;
    torch::Tensor opponent_mask;
    const std::vector<std::unique_ptr<IAgent>> &opponent_pool;
    std::atomic<bool> reset_recently;
    Random &rng;
    std::atomic<float> score;
    std::atomic<bool> should_stop;
    std::atomic<bool> slow;
    bool start_position;
    std::atomic<unsigned long long> *timestep;

  public:
    SingleRolloutGenerator(const IAgent &agent,
                           std::unique_ptr<IEcsEnv> environment,
                           const std::vector<std::unique_ptr<IAgent>> &opponent_pool,
                           IAudioEngine &audio_engine,
                           Random &rng,
                           std::atomic<unsigned long long> *timestep = nullptr);

    void draw(Renderer &renderer, bool lightweight = false) override;
    void fast_forward(unsigned int steps) override;
    cpprl::RolloutStorage generate(unsigned long length) override;
    std::pair<float, float> get_scores() const override;

    inline std::string get_current_opponent() const override
    {
        return opponent->get_name();
    }
    inline const IEcsEnv &get_environment() const override { return *environment; }
    inline void set_fast() override { slow = false; }
    inline void set_slow() override { slow = true; }
    inline void set_timestep_pointer(std::atomic<unsigned long long> *timestep) override
    {
        this->timestep = timestep;
    }
    inline void set_audibility(bool visibility) override
    {
        environment->set_audibility(visibility);
    }
    void stop() override;
};

class MockSingleRolloutGenerator : public trompeloeil::mock_interface<ISingleRolloutGenerator>
{
  public:
    IMPLEMENT_MOCK2(draw);
    IMPLEMENT_MOCK1(fast_forward);
    IMPLEMENT_MOCK1(generate);
    IMPLEMENT_CONST_MOCK0(get_current_opponent);
    IMPLEMENT_CONST_MOCK0(get_environment);
    IMPLEMENT_CONST_MOCK0(get_scores);
    IMPLEMENT_MOCK0(set_fast);
    IMPLEMENT_MOCK0(set_slow);
    IMPLEMENT_MOCK1(set_timestep_pointer);
    IMPLEMENT_MOCK1(set_audibility);
    IMPLEMENT_MOCK0(stop);
};

class SingleRolloutGeneratorFactory
{
  private:
    IAudioEngine &audio_engine;
    Random &rng;

  public:
    SingleRolloutGeneratorFactory(IAudioEngine &audio_engine, Random &rng)
        : audio_engine(audio_engine),
          rng(rng) {}

    std::unique_ptr<ISingleRolloutGenerator> make(
        const IAgent &agent,
        std::unique_ptr<IEcsEnv> environment,
        const std::vector<std::unique_ptr<IAgent>> &opponent_pool,
        std::atomic<unsigned long long> *timestep = nullptr);
};
}