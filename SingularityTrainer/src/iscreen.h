#pragma once

#include <Thor/Input.hpp>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "gui/input.h"

namespace SingularityTrainer
{
class IScreen : IDrawable
{
  public:
    IScreen(){};
    ~IScreen(){};

    virtual void draw(sf::RenderTarget &render_target, bool lightweight = false) = 0;
    virtual void update(const sf::Time &delta_time, const sf::Vector2f &mouse_position, const thor::ActionMap<Inputs> &action_map) = 0;
};
}