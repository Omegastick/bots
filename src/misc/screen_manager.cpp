#include <memory>
#include <stack>

#include <doctest.h>
#include <spdlog/spdlog.h>

#include "screens/iscreen.h"
#include "misc/screen_manager.h"

namespace SingularityTrainer
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
    while (command_queue.size() > 0)
    {
        auto command = command_queue.front();
        switch (command.type)
        {
        case CommandType::Push:
            screens.push(command.screen);
            break;
        case CommandType::Pop:
            screens.pop();
            break;
        }
        command_queue.pop_front();
    }

    if (screens.size() > 0)
    {
        screens.top()->update(delta_time);
    }
}

TEST_CASE("ScreenManager")
{
    struct Screen : public IScreen
    {
        std::function<void()> update_function = [] {};
        std::function<void()> draw_function = [] {};

        virtual void update(const double /*delta_time*/)
        {
            update_function();
        }
        virtual void draw(Renderer & /*renderer*/, bool /*lightweight*/ = false)
        {
            draw_function();
        }
    };
    ScreenManager screen_manager;

    SUBCASE("Update is only called on topmost screen")
    {
        bool lower_update_called = false;
        bool upper_update_called = false;
        Screen lower_screen;
        lower_screen.update_function = [&] { lower_update_called = true; };
        Screen upper_screen;
        upper_screen.update_function = [&] { upper_update_called = true; };
        screen_manager.show_screen(std::make_shared<Screen>(lower_screen));
        screen_manager.show_screen(std::make_shared<Screen>(upper_screen));

        screen_manager.update(1);

        CHECK(!lower_update_called);
        CHECK(upper_update_called);
    }

    SUBCASE("Screens are closed before update is called")
    {
        bool lower_update_called = false;
        bool upper_update_called = false;
        Screen lower_screen;
        lower_screen.update_function = [&] { lower_update_called = true; };
        Screen upper_screen;
        upper_screen.update_function = [&] { upper_update_called = true; };
        screen_manager.show_screen(std::make_shared<Screen>(lower_screen));
        screen_manager.show_screen(std::make_shared<Screen>(upper_screen));

        screen_manager.close_screen();
        screen_manager.update(1);

        CHECK(lower_update_called);
        CHECK(!upper_update_called);
    }

    SUBCASE("Closing then opening a screen behaves correctly")
    {
        bool lower_update_called = false;
        bool upper_update_called = false;
        Screen lower_screen;
        lower_screen.update_function = [&] { lower_update_called = true; };
        Screen upper_screen;
        upper_screen.update_function = [&] { upper_update_called = true; };
        screen_manager.show_screen(std::make_shared<Screen>(lower_screen));
        screen_manager.close_screen();
        screen_manager.show_screen(std::make_shared<Screen>(upper_screen));

        screen_manager.update(1);

        CHECK(!lower_update_called);
        CHECK(upper_update_called);
    }
}
}
