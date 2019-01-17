#include <SFML/Graphics.hpp>
#include <stack>
#include <memory>

#include "iscreen.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
void ScreenManager::show_screen(std::shared_ptr<IScreen> screen)
{
    screens.push(screen);
}
void ScreenManager::close_screen()
{
    screens.pop();
}
void ScreenManager::update(sf::Time delta_time, sf::RenderWindow &window)
{
    sf::Vector2f mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    screens.top()->update(delta_time, mouse_position);
}
void ScreenManager::draw(sf::RenderTarget &render_target)
{
    screens.top()->draw(render_target);
}
}
