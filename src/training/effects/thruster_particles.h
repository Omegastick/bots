#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "training/effects/ieffect.h"

namespace ai
{
class Renderer;

class ThrusterParticles : public IEffect
{
  private:
    glm::vec4 particle_color;
    b2Transform transform;

  public:
    ThrusterParticles(b2Transform transform, glm::vec4 particle_color);

    virtual void trigger(Renderer &renderer);
};
}