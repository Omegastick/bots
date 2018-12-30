#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <Box2d/Box2D.h>

#include "box.h"

int main(int argc, const char *argv[])
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Woop", sf::Style::Default);
    window.setFramerateLimit(60);

    sf::Event event;

    // World init
    b2Vec2 gravity(0.0f, -9.8f);
    b2World world(gravity);

    // Ground
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);
    b2Body *groundBody = world.CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 10.0f);
    groundBody->CreateFixture(&groundBox, 0.0f);

    // Box2D setup
    float timeStep = 1.0f / 60.0f;
    int velocityIterations = 6;
    int positionIterations = 2;

    // Ground sprite
    sf::RectangleShape groundShape(sf::Vector2f(1000.0f, 200.0f));
    groundShape.setOrigin(500.0f, 100.0f);
    groundShape.setFillColor(sf::Color::Green);

    b2Vec2 spriteOffset(-1920.0f / 10 / 2, -1080.0f / 10 / 2);
    float rad2Deg = 180.0f / 3.1415f;

    // Boxes
    std::vector<Box> boxes;
    for (int i = 0; i < 1000; i++)
    {
        b2Vec2 size(1.0f, 1.0f);
        b2Vec2 position(0.0f, i * 3.0f);
        Box box(size, position, world);
        box.body->SetAngularVelocity(1.0f);
        boxes.push_back(box);
    }

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
        world.Step(timeStep, velocityIterations, positionIterations);

        // Ground
        b2Vec2 groundPosition = groundBody->GetPosition() + spriteOffset;
        groundShape.setPosition(-groundPosition.x * 10, -groundPosition.y * 10);

        // Box
        for (auto &box : boxes)
        {
            b2Vec2 boxPosition = box.body->GetPosition() + spriteOffset;
            box.shape.setPosition(-boxPosition.x * 10, -boxPosition.y * 10);
            box.shape.setRotation(box.body->GetAngle() * rad2Deg);
        }

        /*
         *  Draw
         */
        window.clear(sf::Color::Black);

        for (const auto &box : boxes)
        {
            window.draw(box.shape);
        }

        window.draw(groundShape);

        window.display();
    }

    return 0;
}