#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "idrawable.h"

namespace SingularityTrainer
{
class IGUIObject : IDrawable
{
  public:
    IGUIObject(){};
    ~IGUIObject(){};

    virtual void handle_input() = 0;
    virtual void draw(sf::RenderTarget &render_target) = 0;
};
}