#pragma once

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"

namespace SingularityTrainer
{
class IModule;
struct RenderData;

class ModuleLink : IDrawable
{
  public:
    ModuleLink(float x, float y, float rot, IModule *parent);
    ~ModuleLink();

    b2Transform get_global_transform() const;
    virtual RenderData get_render_data(bool lightweight = false);
    void link(ModuleLink &other);
    void snap_to_other(ModuleLink &other);

    bool is_parent;
    bool linked;
    bool visible;
    IModule *linked_module;
    IModule *parent_module;
    ModuleLink *pair_link;
    b2Transform transform;
};
}