#pragma once

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "third_party/di.hpp"
#include "training/environments/ienvironment.h"
#include "networking/action_store.h"
#include "networking/messages.h"

namespace ai
{
class BodyFactory;
class Random;

struct TickResult
{
    std::vector<Transform> agent_transforms;
    std::unordered_map<unsigned int, Transform> entity_transforms;
    std::vector<std::unique_ptr<IEvent>> events;
    std::vector<float> hps;
    std::vector<float> scores;
    bool done;
    int tick;
    int victor;
};

static auto TickLength = [] {};

class Game
{
  private:
    std::unique_ptr<ActionStore> action_store;
    BodyFactory &body_factory;
    std::vector<nlohmann::json> body_specs;
    int current_tick;
    std::unique_ptr<IEnvironment> env;
    IEnvironmentFactory &env_factory;
    double last_tick_time;
    Random &rng;
    double tick_length;

    void setup_env();

  public:
    BOOST_DI_INJECT(Game,
                    (named = TickLength) double tick_length,
                    BodyFactory &body_factory,
                    IEnvironmentFactory &env_factory,
                    Random &rng);

    bool add_body(nlohmann::json body_spec);
    bool ready_to_tick(double current_time);
    void set_action(int tick, int player, const std::vector<int> &action);
    TickResult tick(double current_time);
};
}