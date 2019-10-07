#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "training/effects/ieffect.h"

namespace SingularityTrainer
{
struct RenderData;

class BulletExplosion : public IEffect
{
  private:
    glm::vec4 particle_color;
    b2Vec2 position;

  public:
    BulletExplosion(b2Vec2 position, glm::vec4 particle_color);

    virtual RenderData trigger();
};
}