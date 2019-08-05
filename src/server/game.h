#pragma once

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "training/environments/ienvironment.h"
#include "server/action_store.h"
#include "server/messages.h"

namespace SingularityTrainer
{
class BodyFactory;
class Random;

struct TickResult
{
    std::vector<Transform> agent_transforms;
    std::unordered_map<unsigned int, Transform> entity_transforms;
    bool done;
    int tick;
    int victor;
};

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
    Game(double current_time,
         double tick_length,
         BodyFactory &body_factory,
         IEnvironmentFactory &env_factory,
         Random &rng);

    bool add_body(nlohmann::json body_spec);
    bool ready_to_tick(double current_time);
    void set_action(int tick, int player, const std::vector<int> &action);
    TickResult tick();
};
}