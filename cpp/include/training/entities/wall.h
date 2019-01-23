#pragma once

#include <SFML/Graphics.hpp>

#include "idrawable.h"

namespace SingularityTrainer
{
class Wall : IDrawable
{
  public:
    Wall(){};
    ~Wall(){};

    virtual void draw(sf::RenderTarget &render_target);
};
}