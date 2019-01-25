#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "idrawable.h"
#include "training/actions/iaction.h"
#include "training/modules/module_link.h"
#include "training/agents/iagent.h"

namespace SingularityTrainer
{
class ModuleLink;
class IAction;
class IAgent;

class IModule : IDrawable
{
  public:
    IModule(){};
    ~IModule(){};

    virtual std::vector<IModule *> get_children();
    virtual std::vector<IModule *> get_children(std::vector<IModule *> child_list);
    virtual std::vector<float> get_sensor_reading();
    virtual void draw(sf::RenderTarget &render_target);

    const IModule *root;
    std::vector<ModuleLink> module_links;
    std::vector<std::unique_ptr<IAction>> actions;
    b2Transform transform;
    float rotation_rad;
    b2PolygonShape shape;
    sf::Sprite sprite;
    IAgent *agent;
};
}