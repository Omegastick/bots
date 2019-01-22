#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "idrawable.h"
#include "training/actions/iaction.h"

namespace SingularityTrainer
{
class ModuleLink : IDrawable
{
  public:
    ModuleLink();
    ~ModuleLink();

    void draw(sf::RenderTarget &render_target);

    bool linked;
    IModule *linked_module;
    bool visible;
};

class IModule : IDrawable
{
  public:
    virtual std::vector<IModule *> get_children() = 0;
    virtual void act(std::vector<int> actions) = 0;
    virtual std::vector<float> get_sensor_reading() = 0;
    virtual void draw(sf::RenderTarget &render_target) = 0;

    const IModule *root;
    std::vector<ModuleLink> module_links;
    std::vector<std::unique_ptr<IAction>> actions;
    b2Fixture fixture;
};
}