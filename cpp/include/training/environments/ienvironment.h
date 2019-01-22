#pragma once

#include <SFML/Graphics.hpp>
#include <future>
#include <memory>
#include <vector>

#include "idrawable.h"

namespace SingularityTrainer
{
struct StepInfo
{
    std::vector<float> observation;
    float reward;
    bool done;
};

class IEnvironment : IDrawable
{
  public:
    IEnvironment(){};
    ~IEnvironment(){};

    virtual void start_thread() = 0;
    virtual std::future<std::unique_ptr<StepInfo>> step(std::vector<int> actions, float step_length) = 0;
    virtual void forward(float step_length) = 0;
    virtual std::future<std::unique_ptr<StepInfo>> reset() = 0;
    virtual void change_reward(float reward_delta) = 0;
    virtual void set_done() = 0;
    virtual void draw(sf::RenderTarget &render_target) = 0;
};
}