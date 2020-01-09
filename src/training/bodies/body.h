#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "graphics/colors.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"
#include "training/modules/imodule.h"

namespace ai
{
class IAction;
class IModule;
class IModuleFactory;
class RigidBody;
class Random;
class IEnvironment;

class Body : public ICollidable
{
  private:
    std::vector<IAction *> actions;
    ColorScheme color_scheme;
    bool debug_draw;
    IEnvironment *environment;
    std::vector<std::shared_ptr<IModule>> modules;
    std::string name;
    std::unique_ptr<RigidBody> rigid_body;

    void init_rigid_body();
    void recurse_json_modules(const nlohmann::json &module_json,
                              IModule *parent_module = nullptr,
                              int parent_link = 0,
                              int child_link = 0);

  protected:
    float hp;
    IModuleFactory &module_factory;
    Random *rng;

  public:
    Body(IModuleFactory &module_factory, Random &rng);
    Body(Body &&other);
    Body(const Body &) = delete;
    Body &operator=(Body &&other);

    void act(std::vector<int> action_flags);
    void add_module(const std::shared_ptr<IModule> module);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);
    std::vector<float> get_observation() const;
    int get_input_count() const;
    void draw(Renderer &renderer, bool lightweight = false);
    void hit(float damage);
    void load_json(const nlohmann::json &json);
    void register_actions();
    void reset();
    void set_color(const ColorScheme &color_scheme);
    void set_rigid_body(std::unique_ptr<RigidBody> rigid_body);
    void sub_update();
    nlohmann::json to_json() const;
    void unlink_module(IModule *module);
    void update_body();

    inline const std::vector<IAction *> &get_actions() const { return actions; }
    inline const ColorScheme &get_color_scheme() const { return color_scheme; }
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
  protected:
    IModuleFactory &module_factory;
    Random &rng;

  public:
    BodyFactory(IModuleFactory &module_factory, Random &rng)
        : module_factory(module_factory), rng(rng) {}

    virtual std::unique_ptr<Body> make();
    virtual std::unique_ptr<Body> make(b2World &world, Random &rng);
};
}