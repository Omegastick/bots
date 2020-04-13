#pragma once

#include <tuple>
#include <vector>

#include <nlohmann/json_fwd.hpp>
#include <torch/types.h>
#include <trompeloeil.hpp>

#include "audio/audio_engine.h"
#include "graphics/renderers/renderer.h"
#include "training/training_program.h"

namespace ai
{
struct EcsStepInfo
{
    std::vector<torch::Tensor> observations;
    torch::Tensor reward, done;
    int victor = -1;
};

class IEcsEnv
{
  public:
    virtual ~IEcsEnv() = 0;

    virtual void draw(Renderer &renderer,
                      IAudioEngine &audio_engine,
                      bool lightweight = false) = 0;
    virtual void forward(double step_length) = 0;
    virtual double get_elapsed_time() const = 0;
    virtual std::pair<double, double> get_scores() const = 0;
    virtual bool is_audible() const = 0;
    virtual EcsStepInfo reset() = 0;
    virtual void set_audibility(bool audibility) = 0;
    virtual void set_body(std::size_t index, const nlohmann::json &body_def) = 0;
    virtual void set_reward_config(const RewardConfig &reward_config) = 0;
    virtual EcsStepInfo step(const std::vector<torch::Tensor> &actions, double step_length) = 0;
};

inline IEcsEnv::~IEcsEnv() {}

class MockEcsEnv : public trompeloeil::mock_interface<IEcsEnv>
{
  public:
    IMPLEMENT_MOCK3(draw);
    IMPLEMENT_MOCK1(forward);
    IMPLEMENT_CONST_MOCK0(get_elapsed_time);
    IMPLEMENT_CONST_MOCK0(get_scores);
    IMPLEMENT_CONST_MOCK0(is_audible);
    IMPLEMENT_MOCK0(reset);
    IMPLEMENT_MOCK1(set_audibility);
    IMPLEMENT_MOCK2(set_body);
    IMPLEMENT_MOCK1(set_reward_config);
    IMPLEMENT_MOCK2(step);
};
}
