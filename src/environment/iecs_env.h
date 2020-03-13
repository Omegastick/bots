#pragma once

#include <vector>

#include <nlohmann/json_fwd.hpp>
#include <torch/types.h>

namespace ai
{
class IAudioEngine;
class Renderer;

struct EcsStepInfo
{
    torch::Tensor observations, reward, done;
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
    virtual bool is_audible() const = 0;
    virtual EcsStepInfo reset() = 0;
    virtual void set_audibility(bool audibility) = 0;
    virtual void set_body(std::size_t index, const nlohmann::json &body_def) = 0;
    virtual EcsStepInfo step(std::vector<torch::Tensor> actions, double step_length) = 0;
};

inline IEcsEnv::~IEcsEnv() {}
}
