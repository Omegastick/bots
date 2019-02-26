#pragma once

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"
#include "graphics/render_data.h"
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
    virtual RenderData get_render_data(bool lightweight = false);

    bool linked;
    IModule *parent_module;
    IModule *linked_module;
    ModuleLink *pair_link;
    bool visible;
    b2Transform transform;
};
}