#include <memory>
#include <stack>

#include <spdlog/spdlog.h>

#include "screens/iscreen.h"
#include "misc/screen_manager.h"

namespace SingularityTrainer
{
ScreenManager::ScreenManager() : screens_to_pop(0) {}

void ScreenManager::show_screen(std::shared_ptr<IScreen> screen)
{
    screens.push(screen);
}
void ScreenManager::close_screen()
{
    screens_to_pop += 1;
}
void ScreenManager::update(double delta_time)
{
    for (int i = 0; i < screens_to_pop; ++i)
    {
        screens.pop();
    }
    screens_to_pop = 0;

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
