#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <Thor/Input.hpp>

#include "communicator.h"
#include "gui/colors.h"
#include "gui/input.h"
#include "random.h"
#include "requests.h"
#include "screen_manager.h"
#include "screens/target_env_screen.h"
#include "test_screen/test_screen.h"

using namespace SingularityTrainer;

void on_window_resize(thor::ActionContext<Inputs> context)
{
    sf::RenderWindow *window = static_cast<sf::RenderWindow *>(context.window);
    sf::View view = window->getView();
    sf::Vector2u window_size = context.window->getSize();

    float window_ratio = window_size.x / (float)window_size.y;
    float view_ratio = view.getSize().x / (float)view.getSize().y;
    float size_x = 1;
    float size_y = 1;
    float pos_x = 0;
    float pos_y = 0;

    bool horizontal_spacing = true;
    if (window_ratio < view_ratio)
        horizontal_spacing = false;

    if (horizontal_spacing)
    {
        size_x = view_ratio / window_ratio;
        pos_x = (1 - size_x) / 2.f;
    }
    else
    {
        size_y = window_ratio / view_ratio;
        pos_y = (1 - size_y) / 2.f;
    }

    view.setViewport(sf::FloatRect(pos_x, pos_y, size_x, size_y));

    window->setView(view);
}

int main(int argc, const char *argv[])
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Singularity Trainer", sf::Style::Default);
    window.setFramerateLimit(60);
    sf::View view(sf::FloatRect(0, 0, 1920, 1080));
    window.setView(view);

    sf::Event event;
    sf::Clock frame_clock;

    thor::ActionMap<Inputs> action_map;
    action_map[Inputs::Quit] = thor::Action(sf::Event::Closed);
    action_map[Inputs::ResizeWindow] = thor::Action(sf::Event::Resized);
    thor::ActionMap<Inputs>::CallbackSystem input_callback_system;
    input_callback_system.connect(Inputs::ResizeWindow, std::function<void(thor::ActionContext<Inputs>)>(on_window_resize));

    ScreenManager screen_manager;
    ResourceManager resource_manager = ResourceManager("cpp/assets/");
    std::unique_ptr<Communicator> communicator = std::make_unique<Communicator>("tcp://127.0.0.1:10201");
    std::unique_ptr<Random> rng = std::make_unique<Random>(1);

    std::shared_ptr<TargetEnvScreen> test_screen = std::make_shared<TargetEnvScreen>(resource_manager, communicator.get(), rng.get(), 7);
    // std::shared_ptr<TestScreen> test_screen = std::make_shared<TestScreen>(resource_manager, communicator.get(), 7);
    screen_manager.show_screen(test_screen);

    frame_clock.restart();
    while (window.isOpen())
    {
        /*
         *  Input
         */
        action_map.update(window);
        action_map.invokeCallbacks(input_callback_system, &window);
        if (action_map.isActive(Inputs::Quit))
        {
            while (screen_manager.stack_size() > 0)
            {
                screen_manager.close_screen();
                std::cout << screen_manager.stack_size() << std::endl;
            }
            window.close();
            break;
        }

        /*
         *  Update
         */
        screen_manager.update(frame_clock.restart(), window, action_map);

        /*
         *  Draw
         */
        window.clear(cl_background);
        screen_manager.draw(window);
        window.display();
    }

    return 0;
}
