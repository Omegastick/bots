#pragma once

#include <memory>
#include <vector>

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

class IAgent : public ICollidable
{
  public:
    virtual ~IAgent() = 0;

    virtual void act(std::vector<int> action_flags) = 0;
    virtual std::vector<float> get_observation() = 0;
    virtual void begin_contact(RigidBody *other) = 0;
    virtual void end_contact(RigidBody *other) = 0;
    virtual RenderData get_render_data(bool lightweight = false) = 0;
    virtual void update_body() = 0;
    virtual IEnvironment *get_environment() const = 0;

    std::vector<std::unique_ptr<IModule>> modules;
    std::vector<IAction *> actions;
    std::unique_ptr<RigidBody> rigid_body;
    bool debug_draw;
    Random *rng;
};

inline IAgent::~IAgent() {}
}