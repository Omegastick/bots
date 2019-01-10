#include <SFML/Graphics.hpp>
#include <stack>
#include <memory>

#include "iscreen.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
void ScreenManager::show_screen(std::shared_ptr<IScreen> screen)
{
    screens_.push(screen);
}
void ScreenManager::close_screen()
{
    screens_.pop();
}
void ScreenManager::update(float delta_time)
{
    screens_.top()->update(delta_time);
}
void ScreenManager::draw(sf::RenderTarget &render_target)
{
    screens_.top()->draw(render_target);
}
}
