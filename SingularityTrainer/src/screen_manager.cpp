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
void ScreenManager::update(float delta_time)
{
    screens.top()->update(delta_time);
}
int ScreenManager::stack_size()
{
    return screens.size();
}
void ScreenManager::draw(float delta_time, Renderer &renderer, bool lightweight)
{
    screens.top()->draw(delta_time, renderer, lightweight);
}
}
