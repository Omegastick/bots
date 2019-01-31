#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class IModule;

class ModuleLink : IDrawable
{
  public:
    ModuleLink(float x, float y, float rot, IModule *parent);
    ~ModuleLink();

    void link(ModuleLink *other);
    void draw(sf::RenderTarget &render_target);

    bool linked;
    IModule *parent_module;
    IModule *linked_module;
    ModuleLink *pair_link;
    bool visible;
    b2Transform transform;
};
}