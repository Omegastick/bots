#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <Thor/Input.hpp>

#include "communicator.h"
#include "gui/input.h"
#include "requests.h"
#include "screen_manager.h"
#include "test_screen/test_screen.h"

using namespace SingularityTrainer;

int main(int argc, const char *argv[])
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;

    sf::RenderWindow window(sf::VideoMode(1440, 810), "Singularity Trainer", sf::Style::Default, settings);
    window.setFramerateLimit(60);
    sf::View view(sf::FloatRect(0, 0, 1920, 1080));
    window.setView(view);

    sf::Event event;
    sf::Clock frame_clock;

    thor::ActionMap<Inputs> action_map;
    action_map[Inputs::Quit] = thor::Action(sf::Event::Closed);

    ScreenManager screen_manager;
    std::shared_ptr<ResourceManager> resource_manager = std::make_shared<ResourceManager>();
    std::shared_ptr<Communicator> communicator = std::make_shared<Communicator>("tcp://127.0.0.1:10201");

    std::shared_ptr<TestScreen> test_screen = std::make_shared<TestScreen>(resource_manager, communicator, 7);
    screen_manager.show_screen(test_screen);

    frame_clock.restart();
    while (window.isOpen())
    {
        /*
         *  Process events
         */
        action_map.update(window);
        if (action_map.isActive(Inputs::Quit))
        {
            while (screen_manager.stack_size() > 0)
            {
                screen_manager.close_screen();
                std::cout << screen_manager.stack_size() << std::endl;;
            }
            window.close();
            break;
        }

        /*
         *  Update logic
         */
        screen_manager.update(frame_clock.restart(), window, action_map);

        /*
         *  Draw
         */
        window.clear(sf::Color::Black);

        screen_manager.draw(window);

        window.display();
    }

    return 0;
}
