#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"

namespace SingularityTrainer
{
class IAgent;
class Bullet;

class GunModule : public IModule, public IActivatable
{
  public:
    GunModule(b2Body &body, IAgent *agent);
    ~GunModule();

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void activate();
    virtual void update();

  private:
    int cooldown;
    int steps_since_last_shot;
    std::vector<std::unique_ptr<Bullet>> bullets;
};
}