#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "resource_manager.h"
#include "training/actions/iaction.h"
#include "training/agents/iagent.h"
#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"

namespace SingularityTrainer
{
class ThrusterModule : public IModule, public IActivatable
{
  public:
    ThrusterModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent);
    ~ThrusterModule();

    virtual void draw(sf::RenderTarget &render_target);
    virtual void activate();
    virtual void update();
};
}