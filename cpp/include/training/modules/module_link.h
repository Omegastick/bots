#pragma once

#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class IModule;

class ModuleLink : IDrawable
{
  public:
    ModuleLink(float x, float y, float rot);
    ~ModuleLink();

    void draw(sf::RenderTarget &render_target);

    bool linked;
    IModule *linked_module;
    bool visible;
    float x;
    float y;
    float rot;
};
}