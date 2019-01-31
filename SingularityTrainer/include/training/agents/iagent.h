#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "idrawable.h"
#include "training/actions/iaction.h"
#include "training/icollidable.h"
#include "training/modules/imodule.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class IAction;
class IModule;

class IAgent : public IDrawable, public ICollidable
{
  public:
    IAgent(){};
    ~IAgent(){};

    virtual void act(std::vector<int> action_flags) = 0;
    virtual std::vector<float> get_observation() = 0;
    virtual void begin_contact(RigidBody *other) = 0;
    virtual void end_contact(RigidBody *other) = 0;
    virtual void draw(sf::RenderTarget &render_target) = 0;
    virtual void update_body() = 0;

    std::vector<std::unique_ptr<IModule>> modules;
    std::vector<IAction *> actions;
    std::unique_ptr<RigidBody> rigid_body;
    bool debug_draw;
};
}