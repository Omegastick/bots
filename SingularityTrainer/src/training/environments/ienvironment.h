#pragma once

#include <future>
#include <memory>
#include <vector>

#include "graphics/idrawable.h"

namespace SingularityTrainer
{
class IAgent;

struct StepInfo
{
    std::vector<std::vector<float>> observation;
    std::vector<float> reward;
    std::vector<bool> done;
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
    std::promise<std::unique_ptr<StepInfo>> promise;
    float step_length;
    std::vector<std::vector<int>> actions;
};

class IEnvironment : public IDrawable
{
  public:
    virtual ~IEnvironment() = 0;

    virtual void start_thread() = 0;
    virtual std::future<std::unique_ptr<StepInfo>> step(const std::vector<std::vector<int>> &actions, float step_length) = 0;
    virtual void forward(float step_length) = 0;
    virtual std::future<std::unique_ptr<StepInfo>> reset() = 0;
    virtual void change_reward(int agent, float reward_delta) = 0;
    virtual void change_reward(IAgent *agent, float reward_delta) = 0;
    virtual void set_done() = 0;
    virtual RenderData get_render_data(bool lightweight = false) = 0;
    virtual float get_elapsed_time() const = 0;
};

inline IEnvironment::~IEnvironment() {}
}