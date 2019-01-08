#define ZMQ_STATIC
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <sodium.h>
#include <zmq.hpp>

#include "screen_manager.h"
#include "test_screen.h"

int main(int argc, const char *argv[])
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;

    sf::RenderWindow window(sf::VideoMode(1440, 900), "Singularity Trainer", sf::Style::Default, settings);
    window.setFramerateLimit(360);

    sf::Event event;
    sf::Clock frameClock;

    zmq::context_t context;

    STrainer::TestScreen test_screen(window);
    STrainer::ScreenManager::get_instance().show_screen(&test_screen);

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
        STrainer::ScreenManager::get_instance().update(frameClock.getElapsedTime().asSeconds());
        frameClock.restart();

        /*
         *  Draw
         */
        window.clear(sf::Color::Black);

        STrainer::ScreenManager::get_instance().draw(window);

        window.display();
    }

    return 0;
}
