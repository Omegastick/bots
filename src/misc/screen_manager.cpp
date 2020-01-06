#include <memory>
#include <stack>

#include <doctest.h>
#include <spdlog/spdlog.h>

#include "screens/iscreen.h"
#include "misc/screen_manager.h"

namespace ai
{
ScreenManager::ScreenManager() {}

void ScreenManager::show_screen(std::shared_ptr<IScreen> screen)
{
    command_queue.push_back({CommandType::Push, screen});
}

void ScreenManager::close_screen()
{
    command_queue.push_back({CommandType::Pop});
}

std::shared_ptr<IScreen> ScreenManager::current_screen()
{
    return screens.top();
}

void ScreenManager::draw(Renderer &renderer, bool lightweight)
{
    screens.top()->draw(renderer, lightweight);
}

void ScreenManager::exit()
{
    while (stack_size() > 0)
    {
        screens.pop();
    }
}

int ScreenManager::stack_size()
{
    return screens.size();
}

void ScreenManager::update(double delta_time)
{
    bool new_screen_showing = command_queue.size() > 0;
    while (command_queue.size() > 0)
    {
        auto command = command_queue.front();
        if (command.type == CommandType::Push)
        {
            screens.push(command.screen);
        }
        else // CommandType::Pop
        {
            screens.pop();
        }
        command_queue.pop_front();
    }

    if (screens.size() > 0)
    {
        if (new_screen_showing)
        {
            screens.top()->on_show();
        }
        screens.top()->update(delta_time);
    }
}

using trompeloeil::_;

TEST_CASE("ScreenManager")
{
    ScreenManager screen_manager;

    SUBCASE("Update is only called on topmost screen")
    {
        auto lower_screen = std::make_shared<MockScreen>();
        auto upper_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*upper_screen, on_show());
        REQUIRE_CALL(*upper_screen, update(_));
        screen_manager.show_screen(lower_screen);
        screen_manager.show_screen(upper_screen);

        screen_manager.update(1);
    }

    SUBCASE("Screens are closed before update is called")
    {
        auto lower_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*lower_screen, on_show());
        REQUIRE_CALL(*lower_screen, update(_));
        auto upper_screen = std::make_shared<MockScreen>();
        screen_manager.show_screen(lower_screen);
        screen_manager.show_screen(upper_screen);

        screen_manager.close_screen();
        screen_manager.update(1);
    }

    SUBCASE("Closing then opening a screen in the same tick only calls update on the new screen")
    {
        auto lower_screen = std::make_shared<MockScreen>();
        auto upper_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*upper_screen, on_show());
        REQUIRE_CALL(*upper_screen, update(_));
        screen_manager.show_screen(lower_screen);
        screen_manager.close_screen();
        screen_manager.show_screen(upper_screen);

        screen_manager.update(1);
    }

    SUBCASE("on_show() is not called after the first update in which a screen is shown")
    {
        auto screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*screen, on_show());
        REQUIRE_CALL(*screen, update(_));
        REQUIRE_CALL(*screen, update(_));
        screen_manager.show_screen(screen);
        screen_manager.update(1);
        screen_manager.update(1);
    }

    SUBCASE("When a screen is closed, the on_show() method of the screen underneath it is"
            "called")
    {
        auto lower_screen = std::make_shared<MockScreen>();
        auto upper_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*upper_screen, on_show());
        REQUIRE_CALL(*upper_screen, update(_));
        REQUIRE_CALL(*lower_screen, on_show());
        REQUIRE_CALL(*lower_screen, update(_));
        screen_manager.show_screen(lower_screen);
        screen_manager.show_screen(upper_screen);
        screen_manager.update(1);
        screen_manager.close_screen();
        screen_manager.update(1);
    }
}
}
