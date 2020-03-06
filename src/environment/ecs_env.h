#pragma once

#include <entt/entt.hpp>

#include "environment/iecs_env.h"

namespace ai
{
class EcsEnv : public IEcsEnv
{
  private:
    entt::registry registry;

  public:
    EcsEnv();

    void draw(Renderer &renderer, IAudioEngine &audio_engine, bool lightweight = false) override;
    void forward(double step_length) override;
    double get_elapsed_time() const override;
    bool is_audible() const override;
    EcsStepInfo reset() override;
    void set_audibility(bool audibility) override;
    EcsStepInfo step(std::vector<torch::Tensor> actions, double step_length) override;
};
}