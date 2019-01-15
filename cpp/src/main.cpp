#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "communicator.h"
#include "requests.h"
#include "screen_manager.h"
#include "test_screen/test_screen.h"

using namespace SingularityTrainer;

int main(int argc, const char *argv[])
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;

    sf::RenderWindow window(sf::VideoMode(1440, 810), "Singularity Trainer", sf::Style::Default, settings);
    // window.setFramerateLimit(60);
    sf::View view(sf::FloatRect(0, 0, 1920, 1080));
    window.setView(view);

    sf::Event event;
    sf::Clock frameClock;

    ScreenManager screen_manager;
    std::shared_ptr<ResourceManager> resource_manager = std::make_shared<ResourceManager>();
    std::shared_ptr<Communicator> communicator = std::make_shared<Communicator>("tcp://127.0.0.1:10201");

    std::shared_ptr<TestScreen> test_screen = std::make_shared<TestScreen>(resource_manager, communicator, 7);
    screen_manager.show_screen(test_screen);

    frameClock.restart();
    while (window.isOpen())
    {
        /*
         *  Process events
         */
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        /*
         *  Update logic
         */
        screen_manager.update(frameClock.getElapsedTime().asSeconds());
        frameClock.restart();

        /*
         *  Draw
         */
        window.clear(sf::Color::Black);

        screen_manager.draw(window);

        window.display();
    }

    return 0;
}
