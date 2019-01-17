#include <SFML/Graphics.hpp>
#include <iostream>

#include "gui/gui_panel.h"

namespace SingularityTrainer
{
GUIPanel::GUIPanel(float x, float y, float width, float height) : mouse_over(false)
{
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(sf::Color::Red);
}

GUIPanel::~GUIPanel() {}

void GUIPanel::handle_input(const sf::Vector2f &mouse_position, const thor::ActionMap<Inputs> &action_map)
{
    if (shape.getGlobalBounds().contains(mouse_position.x, mouse_position.y))
    {
        if (!mouse_over)
        {
            mouse_over = true;
            shape.setFillColor(sf::Color::Cyan);
        }

        for (auto child : children)
        {
            child->handle_input(mouse_position, action_map);
        }
    }
    else
    {
        if (mouse_over)
        {
            mouse_over = false;
            shape.setFillColor(sf::Color::Red);
        }
    }
}

void GUIPanel::draw(sf::RenderTarget &render_target)
{
    render_target.draw(shape);
}
}