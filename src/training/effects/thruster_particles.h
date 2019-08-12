#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "training/effects/ieffect.h"

namespace SingularityTrainer
{
class Random;
class RenderData;

class ThrusterParticles : public IEffect
{
  private:
    glm::vec4 particle_color;
    Random &rng;
    b2Transform transform;

  public:
    ThrusterParticles(b2Transform transform, glm::vec4 particle_color, Random &rng);

    virtual RenderData trigger();
};
}