#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Box2D/Box2D.h>
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
    float hp;
    std::vector<std::shared_ptr<IModule>> modules;
    std::vector<IAction *> actions;
    std::unique_ptr<RigidBody> rigid_body;
    bool debug_draw;
    Random *rng;
    IEnvironment *environment;
    std::string name;

    void init_rigid_body();
    void recurse_json_modules(const nlohmann::json &module_json, IModule *parent_module = nullptr, int parent_link = 0, int child_link = 0);

  public:
    Agent(Random &rng);
    Agent(Agent &&other);
    Agent(const Agent &) = delete;
    Agent &operator=(Agent &&other);

    void act(std::vector<int> action_flags);
    void add_module(const std::shared_ptr<IModule> module);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);
    std::vector<float> get_observation();
    RenderData get_render_data(bool lightweight = false);
    void hit(float damage);
    void load_json(const nlohmann::json &json);
    void register_actions();
    void reset();
    void set_rigid_body(std::unique_ptr<RigidBody> rigid_body);
    nlohmann::json to_json() const;
    void unlink_module(std::shared_ptr<IModule> module);
    void update_body();

    inline const std::vector<IAction *> &get_actions() const { return actions; }
    inline IEnvironment *get_environment() const { return environment; }
    inline float get_hp() const { return hp; }
    inline const std::vector<std::shared_ptr<IModule>> &get_modules() const { return modules; }
    inline std::string &get_name() { return name; }
    inline RigidBody *get_rigid_body() const { return rigid_body.get(); }
    inline Random *get_rng() const { return rng; }
    inline void set_environment(IEnvironment &environment) { this->environment = &environment; }
    inline void set_name(std::string &name) { this->name = name; }
    inline void set_rng(Random &rng) { this->rng = &rng; }
};

class AgentFactory
{
  public:
    virtual std::unique_ptr<Agent> make(Random &rng)
    {
        return std::make_unique<Agent>(rng);
    }

    virtual std::unique_ptr<Agent> make(b2World &world, Random &rng)
    {
        auto agent = std::make_unique<Agent>(rng);
        auto rigid_body = std::make_unique<RigidBody>(
            b2_dynamicBody,
            b2Vec2_zero,
            world,
            nullptr,
            RigidBody::ParentTypes::Agent);
        agent->set_rigid_body(std::move(rigid_body));
        return agent;
    }
};
}