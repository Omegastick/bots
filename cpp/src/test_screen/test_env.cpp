#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

#include "test_screen/bot.h"
#include "test_screen/test_env.h"
#include "test_screen/wall.h"

namespace SingularityTrainer
{
class ContactListener : public b2ContactListener
{
    void BeginContact(b2Contact *contact)
    {
        void *user_data_a = contact->GetFixtureA()->GetBody()->GetUserData();
        void *user_data_b = contact->GetFixtureB()->GetBody()->GetUserData();

        if (user_data_a && user_data_b)
        {
            RigidBody *body_a = static_cast<RigidBody *>(user_data_a);
            RigidBody *body_b = static_cast<RigidBody *>(user_data_b);
            // body_a->begin_contact(*body_b);
            // body_b->begin_contact(*body_a);
            std::cout << "Hi\n";
        }
    }

    void EndContact(b2Contact *contact)
    {
        void *user_data_a = contact->GetFixtureA()->GetBody()->GetUserData();
        void *user_data_b = contact->GetFixtureB()->GetBody()->GetUserData();

        if (user_data_a && user_data_b)
        {
            RigidBody *body_a = static_cast<RigidBody *>(user_data_a);
            RigidBody *body_b = static_cast<RigidBody *>(user_data_b);
            // body_a->end_contact(*body_b);
            // body_b->end_contact(*body_a);
        }
    }
};

TestEnv::TestEnv(std::shared_ptr<ResourceManager> resource_manager, float x, float y, float scale)
{
    // Box2D world
    world = std::make_unique<b2World>(b2Vec2(0, 0));
    contact_listener = std::make_unique<ContactListener>();
    world->SetContactListener(contact_listener.get());

    // Bot
    bot = std::make_unique<Bot>(resource_manager, *world);

    // Walls
    walls.push_back(std::make_unique<Wall>(-5, -5, 10, 0.1, *world));
    walls.push_back(std::make_unique<Wall>(-5, -5, 0.1, 10, *world));
    walls.push_back(std::make_unique<Wall>(-5, 4.9, 10, 0.1, *world));
    walls.push_back(std::make_unique<Wall>(4.9, -5, 0.1, 10, *world));

    // Display
    render_texture.create(1000, 1000);
    sprite.setTexture(render_texture.getTexture());
    sprite.setPosition(x, y);
    sprite.setScale(scale, scale);

    render_texture.setView(sf::View(sf::Vector2f(0, 0), sf::Vector2f(10, 10)));
}

TestEnv::~TestEnv(){};

void TestEnv::draw(sf::RenderTarget &render_target)
{
    // Draw onto temporary texture
    render_texture.clear();
    bot->draw(render_texture);
    for (auto &wall : walls)
    {
        wall->draw(render_texture);
    }
    render_texture.display();

    // Draw temporary tecture onto window
    sprite.setTexture(render_texture.getTexture());
    render_target.draw(sprite);
}

std::unique_ptr<StepInfo> TestEnv::step(std::vector<bool> &actions)
{
    // Act
    bot->act(actions);

    // Step simulation
    world->Step(1.f / 60.f, 6, 4);

    // Return step information
    std::unique_ptr<StepInfo> step_info = std::make_unique<StepInfo>();
    step_info->observation = bot->get_observation();

    return step_info;
}
}