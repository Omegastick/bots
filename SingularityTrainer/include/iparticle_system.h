#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "idrawable.h"

namespace SingularityTrainer
{
class IParticleSystem : public IDrawable
{
  public:
    IParticleSystem(){};
    virtual ~IParticleSystem() = 0;

    virtual void update(float delta_time) = 0;
    virtual void draw(sf::RenderTarget &render_target, bool lightweight = false) = 0;

  private:
    IParticleSystem &operator=(const IParticleSystem &) = default;
};

inline IParticleSystem::~IParticleSystem() {}
}