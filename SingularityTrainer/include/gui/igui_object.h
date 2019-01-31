#pragma once

#include <SFML/Graphics.hpp>
#include <Thor/Input.hpp>
#include <memory>

#include "gui/input.h"
#include "idrawable.h"

namespace SingularityTrainer
{
class IGUIObject : IDrawable
{
  public:
    IGUIObject(){};
    ~IGUIObject(){};

    virtual void handle_input(const sf::Vector2f &mouse_position, const thor::ActionMap<Inputs> &action_map) = 0;
    virtual void draw(sf::RenderTarget &render_target) = 0;
};
}