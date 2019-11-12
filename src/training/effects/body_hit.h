#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "training/effects/ieffect.h"

namespace SingularityTrainer
{
class Renderer;

class BodyHit : public IEffect
{
  private:
    glm::vec4 particle_color;
    b2Vec2 position;

  public:
    BodyHit(b2Vec2 position, glm::vec4 particle_color);

    virtual void trigger(Renderer &renderer);
};
}