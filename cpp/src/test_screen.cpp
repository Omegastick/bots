#include <SFML/Graphics.hpp>

#include "test_screen.h"

namespace STrainer
{
TestScreen::TestScreen(sf::RenderTarget &window)
{
    square = sf::RectangleShape(sf::Vector2f(500.0f, 500.0f));
    square.setFillColor(sf::Color::Red);
    square.setRotation(45.0f);
    square.setOrigin(square.getSize().x / 2.0f, square.getSize().y / 2.0f);
    square.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
}
TestScreen::~TestScreen() {}

void TestScreen::draw(sf::RenderTarget &render_target)
{
    render_target.draw(square);
}
void TestScreen::update(float delta_time)
{
    square.rotate(delta_time * 5);
}
}