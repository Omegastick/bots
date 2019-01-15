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
    sf::View view(sf::FloatRect(0, 0, 1920, 1080));
    window.setView(view);

    sf::Event event;
    sf::Clock frame_clock;

    ScreenManager screen_manager;
    std::shared_ptr<ResourceManager> resource_manager = std::make_shared<ResourceManager>();
    std::shared_ptr<Communicator> communicator = std::make_shared<Communicator>("tcp://127.0.0.1:10201");

    std::shared_ptr<TestScreen> test_screen = std::make_shared<TestScreen>(resource_manager, communicator, 7);
    screen_manager.show_screen(test_screen);

    sf::Time time_since_last_display = sf::Time::Zero;

    frame_clock.restart();
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
        sf::Time frame_time = frame_clock.restart();
        screen_manager.update(frame_time.asSeconds());

        /*
         *  Draw
         */
        time_since_last_display += frame_time;
        if (time_since_last_display.asSeconds() > 1.f / 60.f)
        {
            window.clear(sf::Color::Black);

            screen_manager.draw(window);

            window.display();
            time_since_last_display = sf::Time::Zero;
        }
    }

    return 0;
}
