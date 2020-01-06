#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "graphics/render_data.h"
#include "third_party/di.hpp"
#include "training/environments/ienvironment.h"
#include "training/events/ievent.h"

namespace ai
{
const auto TickLength = [] {};

class PlaybackEnv
{
  private:
    double current_tick;
    std::unique_ptr<IEnvironment> env;
    std::vector<EnvState> states;
    std::vector<std::unique_ptr<IEvent>> events;
    double tick_length;

  public:
    BOOST_DI_INJECT(PlaybackEnv,
                    std::unique_ptr<IEnvironment> env,
                    (named = TickLength) double tick_length);

    void add_events(std::vector<std::unique_ptr<IEvent>> events);
    void add_new_state(EnvState state);
    void draw(Renderer &renderer, bool lightweight = false);
    void reset();
    void set_bodies(const std::vector<nlohmann::json> &body_specs);
    void update(double delta_time);

    inline std::vector<Body *> get_bodies() { return env->get_bodies(); }
    inline double get_elapsed_time() { return current_tick * tick_length; }
    inline IEnvironment &get_env() { return *env; }
    inline std::vector<float> get_scores() { return env->get_scores(); }
};
}