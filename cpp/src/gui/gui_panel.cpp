#include <SFML/Graphics.hpp>

#include "gui/gui_panel.h"

namespace SingularityTrainer
{
GUIPanel::GUIPanel(float x, float y, float width, float height)
{
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(sf::Color::Red);
}

GUIPanel::~GUIPanel() {};

void GUIPanel::handle_input()
{
    if (shape.getGlobalBounds().contains(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y))
    {
        shape.setFillColor(sf::Color::Cyan);

        for (auto child : children)
        {
            child->handle_input();
        }
    }
}

void GUIPanel::draw(sf::RenderTarget &render_target)
{
    render_target.draw(shape);
}
}