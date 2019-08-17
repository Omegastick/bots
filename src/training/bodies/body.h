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

class Body : public ICollidable
{
  private:
    std::vector<std::shared_ptr<IModule>> modules;
    std::vector<IAction *> actions;
    std::unique_ptr<RigidBody> rigid_body;
    bool debug_draw;
    IEnvironment *environment;
    std::string name;

    void init_rigid_body();
    void recurse_json_modules(const nlohmann::json &module_json, IModule *parent_module = nullptr, int parent_link = 0, int child_link = 0);

  protected:
    float hp;
    Random *rng;

  public:
    Body(Random &rng);
    Body(Body &&other);
    Body(const Body &) = delete;
    Body &operator=(Body &&other);

    void act(std::vector<int> action_flags);
    void add_module(const std::shared_ptr<IModule> module);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);
    std::vector<float> get_observation() const;
    int get_input_count() const;
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
    inline RigidBody &get_rigid_body() const { return *rigid_body; }
    inline Random &get_rng() const { return *rng; }
    inline void set_environment(IEnvironment &environment) { this->environment = &environment; }
    inline void set_hp(float hp) { this->hp = hp; }
    inline void set_name(std::string &name) { this->name = name; }
    inline void set_rng(Random &rng) { this->rng = &rng; }
};

class BodyFactory
{
  private:
    Random &rng;

  public:
    BodyFactory(Random &rng)
        : rng(rng) {}

    virtual std::unique_ptr<Body> make()
    {
        return std::make_unique<Body>(rng);
    }

    virtual std::unique_ptr<Body> make(b2World &world, Random &rng)
    {
        auto body = std::make_unique<Body>(rng);
        auto rigid_body = std::make_unique<RigidBody>(
            b2_dynamicBody,
            b2Vec2_zero,
            world,
            nullptr,
            RigidBody::ParentTypes::Body);
        body->set_rigid_body(std::move(rigid_body));
        return body;
    }
};
}