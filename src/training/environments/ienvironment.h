#pragma once

#include <future>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <torch/torch.h>

#include "graphics/idrawable.h"
#include "misc/random.h"

namespace SingularityTrainer
{
class Agent;
class RewardConfig;

struct StepInfo
{
    torch::Tensor observation, reward, done;
};

enum Commands
{
    Step,
    Forward,
    Reset,
    Stop
};

struct ThreadCommand
{
    Commands command;
    std::promise<StepInfo> promise;
    float step_length;
    torch::Tensor actions;
};

class IEnvironment : public IDrawable
{
  public:
    virtual ~IEnvironment() = 0;

    virtual void start_thread() = 0;
    virtual std::future<StepInfo> step(torch::Tensor actions, float step_length) = 0;
    virtual void forward(float step_length) = 0;
    virtual std::future<StepInfo> reset() = 0;
    virtual void change_reward(int agent, float reward_delta) = 0;
    virtual void change_reward(Agent *agent, float reward_delta) = 0;
    virtual void set_done() = 0;
    virtual std::vector<Agent *> get_agents() = 0;
    virtual float get_elapsed_time() const = 0;
    virtual RenderData get_render_data(bool lightweight = false) = 0;
    virtual RewardConfig &get_reward_config() = 0;
    virtual b2World &get_world() = 0;
};

inline IEnvironment::~IEnvironment() {}

class IEnvironmentFactory
{
  public:
    virtual ~IEnvironmentFactory() = 0;

    virtual std::unique_ptr<IEnvironment> make(std::unique_ptr<Random> rng,
                                               std::unique_ptr<b2World> world,
                                               std::vector<std::unique_ptr<Agent>> agents,
                                               RewardConfig reward_config) = 0;
};

inline IEnvironmentFactory::~IEnvironmentFactory() {}
}