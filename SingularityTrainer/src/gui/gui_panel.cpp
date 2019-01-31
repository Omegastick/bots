#include <SFML/Graphics.hpp>
#include <iostream>

#include "gui/gui_panel.h"
#include "gui/colors.h"

namespace SingularityTrainer
{
GUIPanel::GUIPanel(float x, float y, float width, float height) : mouse_over(false)
{
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(cl_dark_neutral);
}

GUIPanel::~GUIPanel() {}

void GUIPanel::handle_input(const sf::Vector2f &mouse_position, const thor::ActionMap<Inputs> &action_map)
{
    if (shape.getGlobalBounds().contains(mouse_position.x, mouse_position.y))
    {
        if (!mouse_over)
        {
            mouse_over = true;
            shape.setFillColor(cl_light_neutral);
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
            shape.setFillColor(cl_dark_neutral);
        }
    }
}

void GUIPanel::draw(sf::RenderTarget &render_target)
{
    render_target.draw(shape);
}
}