#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>

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
void ScreenManager::update(sf::Time delta_time, sf::RenderWindow &window, const thor::ActionMap<Inputs> &action_map)
{
    sf::Vector2f mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    screens.top()->update(delta_time, mouse_position, action_map);
}
int ScreenManager::stack_size()
{
    return screens.size();
}
void ScreenManager::draw(sf::RenderTarget &render_target)
{
    render_target.clear(sf::Color::Blue);
    screens.top()->draw(render_target);
}
}
