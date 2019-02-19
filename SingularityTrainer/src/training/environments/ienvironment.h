#pragma once

#include <future>
#include <memory>
#include <vector>

#include "graphics/render_data.h"
#include "graphics/idrawable.h"

namespace SingularityTrainer
{
struct StepInfo
{
    std::vector<float> observation;
    float reward;
    bool done;
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
    std::vector<int> actions;
};

class IEnvironment : public IDrawable
{
  public:
    IEnvironment(){};
    ~IEnvironment(){};

    virtual void start_thread() = 0;
    virtual std::future<std::unique_ptr<StepInfo>> step(std::vector<int> &actions, float step_length) = 0;
    virtual void forward(float step_length) = 0;
    virtual std::future<std::unique_ptr<StepInfo>> reset() = 0;
    virtual void change_reward(float reward_delta) = 0;
    virtual void set_done() = 0;
    virtual RenderData get_render_data(bool lightweight = false) = 0;
};
}