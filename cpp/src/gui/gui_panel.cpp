#include <SFML/Graphics.hpp>
#include <iostream>

#include "gui/gui_panel.h"

namespace SingularityTrainer
{
GUIPanel::GUIPanel(float x, float y, float width, float height)
{
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(sf::Color::Red);
}

GUIPanel::~GUIPanel() {}

void GUIPanel::handle_input(sf::RenderWindow &window)
{
    sf::Vector2f mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    if (shape.getGlobalBounds().contains(mouse_position.x, mouse_position.y))
    {
        shape.setFillColor(sf::Color::Cyan);

        for (auto child : children)
        {
            child->handle_input(window);
        }
    }
    // else
    // {
    //     shape.setFillColor(sf::Color::Red);
    // }
}

void GUIPanel::draw(sf::RenderTarget &render_target)
{
    render_target.draw(shape);
}
}