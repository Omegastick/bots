#include <memory>
#include <stack>

#include <spdlog/spdlog.h>

#include "screens/iscreen.h"
#include "misc/screen_manager.h"

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
void ScreenManager::update(double delta_time)
{
    screens.top()->update(delta_time);
}
int ScreenManager::stack_size()
{
    return screens.size();
}
void ScreenManager::draw(Renderer &renderer, bool lightweight)
{
    screens.top()->draw(renderer, lightweight);
}
}
