#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "resource_manager.h"
#include "training/actions/iaction.h"
#include "training/modules/imodule.h"
#include "training/modules/interfaces/ishootable.h"
#include "training/agents/iagent.h"

namespace SingularityTrainer
{
class GunModule : public IModule, public IShootable
{
  public:
    GunModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent);
    ~GunModule();

    virtual void shoot();

  private:
    int cooldown;
    int steps_since_last_shot;
};
}