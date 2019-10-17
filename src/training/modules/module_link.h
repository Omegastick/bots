#pragma once

#include <Box2D/Box2D.h>

namespace SingularityTrainer
{
class IModule;
class Renderer;

class ModuleLink
{
  public:
    ModuleLink(float x, float y, float rot, IModule *parent);
    ~ModuleLink();

    b2Transform get_global_transform() const;
    void draw(Renderer &renderer, bool lightweight = false);
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