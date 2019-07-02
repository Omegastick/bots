#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <torch/torch.h>

#include "graphics/idrawable.h"
#include "misc/random.h"

namespace SingularityTrainer
{
class Body;
class RewardConfig;

struct StepInfo
{
    std::vector<torch::Tensor> observation;
    torch::Tensor reward, done;
};

class IEnvironment : public IDrawable
{
  public:
    virtual ~IEnvironment() = 0;

    virtual void change_reward(int body, float reward_delta) = 0;
    virtual void change_reward(Body *body, float reward_delta) = 0;
    virtual void forward(float step_length) = 0;
    virtual std::vector<Body *> get_bodies() = 0;
    virtual float get_elapsed_time() const = 0;
    virtual RenderData get_render_data(bool lightweight = false) = 0;
    virtual RewardConfig &get_reward_config() = 0;
    virtual std::vector<float> get_total_rewards() = 0;
    virtual b2World &get_world() = 0;
    virtual StepInfo reset() = 0;
    virtual void set_done() = 0;
    virtual StepInfo step(std::vector<torch::Tensor> actions, float step_length) = 0;
};

inline IEnvironment::~IEnvironment() {}

class IEnvironmentFactory
{
  public:
    virtual ~IEnvironmentFactory() = 0;

    virtual int get_num_bodies() = 0;
    virtual std::unique_ptr<IEnvironment> make(std::unique_ptr<Random> rng,
                                               std::unique_ptr<b2World> world,
                                               std::vector<std::unique_ptr<Body>> bodies,
                                               RewardConfig reward_config) = 0;
};

inline IEnvironmentFactory::~IEnvironmentFactory() {}
}