#pragma once

#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "training/icollidable.h"
#include "training/rigid_body.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class IAction;
class IModule;
class RenderData;
class RigidBody;
class Random;
class IEnvironment;

class Agent : public ICollidable
{
  private:
    std::vector<std::shared_ptr<IModule>> modules;
    std::vector<IAction *> actions;
    std::unique_ptr<RigidBody> rigid_body;
    bool debug_draw;
    Random *rng;
    float hp;
    IEnvironment *environment;

  public:
    Agent(b2World &world, Random *rng, IEnvironment &environment);
    Agent(b2World &world, Random *rng, IEnvironment &environment, const nlohmann::json &json);

    void act(std::vector<int> action_flags);
    void add_module(const std::shared_ptr<IModule> module);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);
    std::vector<float> get_observation();
    RenderData get_render_data(bool lightweight = false);
    void hit(float damage);
    void init_rigid_body(b2World &world);
    void register_actions();
    void reset();
    nlohmann::json to_json() const;
    void update_body();

    inline IEnvironment *get_environment() const { return environment; }
    inline float get_hp() const { return hp; }
    inline RigidBody *get_rigid_body() const { return rigid_body.get(); }
    inline Random *get_rng() const { return rng; }
};
}