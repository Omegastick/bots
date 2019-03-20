#pragma once

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"

namespace SingularityTrainer
{
class IModule;
class RenderData;

class ModuleLink : IDrawable
{
  public:
    ModuleLink(float x, float y, float rot, IModule *parent);
    ~ModuleLink();

    void link(ModuleLink *other);
    virtual RenderData get_render_data(bool lightweight = false);

    bool is_parent;
    bool linked;
    IModule *linked_module;
    IModule *parent_module;
    ModuleLink *pair_link;
    b2Transform transform;
    bool visible;
};
}