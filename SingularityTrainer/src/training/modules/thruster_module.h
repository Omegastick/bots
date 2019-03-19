#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"

namespace SingularityTrainer
{
class Agent;
class RenderData;

class ThrusterModule : public IModule, public IActivatable
{
  private:
    bool active;
    glm::vec4 particle_color;

  public:
    ThrusterModule();

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void activate();
    virtual void update();
};
}