#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "graphics/render_data.h"
#include "third_party/di.hpp"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
const auto TickLength = [] {};

class PlaybackEnv
{
  private:
    double current_tick;
    std::unique_ptr<IEnvironment> env;
    std::vector<EnvState> states;
    double tick_length;

  public:
    BOOST_DI_INJECT(PlaybackEnv,
                    std::unique_ptr<IEnvironment> env,
                    (named = TickLength) double tick_length);

    void add_new_state(EnvState state);
    RenderData get_render_data(bool lightweight = false);
    void reset();
    void set_bodies(const std::vector<nlohmann::json> &body_specs);
    void update(double delta_time);

    inline std::vector<Body *> get_bodies() { return env->get_bodies(); }
    inline IEnvironment &get_env() { return *env; }
    inline std::vector<float> get_scores() { return env->get_scores(); }
};
}