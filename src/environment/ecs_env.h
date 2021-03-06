#pragma once

#include <array>

#include <entt/entt.hpp>
#include <nlohmann/json_fwd.hpp>

#include "environment/iecs_env.h"

namespace ai
{
class EcsEnv : public IEcsEnv
{
  private:
    bool audible;
    std::array<entt::entity, 2> bodies;
    double elapsed_time;
    double game_length;
    entt::registry registry;

  public:
    EcsEnv(double game_length = 60.f);
    EcsEnv(const EcsEnv &) = delete;
    EcsEnv(EcsEnv &&) = delete;

    void draw(Renderer &renderer, IAudioEngine &audio_engine, bool lightweight = false) override;
    void forward(double step_length) override;
    double get_elapsed_time() const override;
    std::pair<double, double> get_scores() const override;
    bool is_audible() const override;
    EcsStepInfo reset() override;
    void set_audibility(bool audibility) override;
    void set_body(std::size_t index, const nlohmann::json &body_def) override;
    void set_reward_config(const RewardConfig &reward_config) override;
    EcsStepInfo step(const std::vector<torch::Tensor> &actions, double step_length) override;
};
}