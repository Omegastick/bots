#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "communicator.h"
#include "requests.h"
#include "screen_manager.h"
#include "test_screen.h"

using namespace SingularityTrainer;

int main(int argc, const char *argv[])
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;

    sf::RenderWindow window(sf::VideoMode(1440, 900), "Singularity Trainer", sf::Style::Default, settings);
    window.setFramerateLimit(360);

    sf::Event event;
    sf::Clock frameClock;

    ScreenManager screenManager;
    ResourceManager resource_manager;
    Communicator communicator("tcp://127.0.0.1:10201");

    TestScreen test_screen(window, resource_manager, communicator);
    screenManager.show_screen(&test_screen);

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
        screenManager.update(frameClock.getElapsedTime().asSeconds());
        frameClock.restart();

        /*
         *  Draw
         */
        window.clear(sf::Color::Black);

        screenManager.draw(window);

        window.display();
    }

    return 0;
}
