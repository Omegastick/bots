#pragma once

#include <Box2D/Box2D.h>
#include <memory>
#include <vector>
#include <glm/vec4.hpp>

#include "graphics/render_data.h"
#include "resource_manager.h"
#include "training/actions/iaction.h"
#include "training/agents/iagent.h"
#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"

namespace SingularityTrainer
{
class ThrusterModule : public IModule, public IActivatable
{
  private:
    bool active;
    glm::vec4 particle_color;

  public:
    ThrusterModule(b2Body &body, IAgent *agent);
    ~ThrusterModule();

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void activate();
    virtual void update();
};
}