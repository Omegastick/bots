#pragma once

#include <SFML/Graphics.hpp>

namespace SingularityTrainer
{
class IDrawable
{
  public:
    IDrawable(){};
    ~IDrawable(){};

    virtual void draw(sf::RenderTarget &render_target, bool lightweight = false) = 0;
};
}