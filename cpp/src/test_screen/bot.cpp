#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <math.h>
#include <memory>
#include <string>
#include <vector>

#include "test_screen/bot.h"
#include "test_screen/rigid_body.h"

inline float rad_to_deg(float radians)
{
    return radians * (180.f / M_PI);
}

namespace SingularityTrainer
{
Bot::Bot(const std::shared_ptr<ResourceManager> resource_manager, b2World &world)
{
    // Sprite
    sprite.setTexture(*resource_manager->texture_store.get("arrow"));
    sprite.setScale(0.25, 0.25);
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

    // Rigidbody
    body_def.type = b2_dynamicBody;
    body_def.position.Set(0, 0);
    body = world.CreateBody(&body_def);
    b2Vec2 vertices[3];
    vertices[0].Set(0, -0.7);
    vertices[1].Set(0.49, 0.74);
    vertices[2].Set(-0.49, 0.74);
    polygon_shape.Set(vertices, 3);
    fixture_def.shape = &polygon_shape;
    fixture_def.density = 1.0;
    fixture_def.friction = 1.0;
    body->CreateFixture(&fixture_def);

    // Labels
    labels = std::vector<std::string>{"bot"};
}

Bot::~Bot() {}

void Bot::draw(sf::RenderTarget &render_target)
{
    b2Vec2 world_position = body->GetPosition();
    sf::Vector2f screen_position(world_position.x * 100, world_position.y * 100);
    sprite.setPosition(screen_position);
    sprite.setRotation(rad_to_deg(body->GetAngle()));
    render_target.draw(sprite);
}

void Bot::act(std::vector<bool> actions)
{
    if (actions[0])
    {
        float angle = body->GetAngle();
        b2Vec2 force(std::sin(angle) * 0.1, std::cos(angle) * 0.1);
        body->ApplyForceToCenter(force, true);
    }
    if (actions[1])
    {
        float angle = body->GetAngle();
        b2Vec2 force(std::sin(angle) * -0.1, std::cos(angle) * -0.1);
        body->ApplyForceToCenter(force, true);
    }
    if (actions[2])
    {
        body->ApplyTorque(0.01, true);
    }
    if (actions[3])
    {
        body->ApplyTorque(-0.01, true);
    }
}

std::vector<float> Bot::get_observation()
{
    std::vector<float> observation;

    b2Vec2 position = body->GetPosition();
    observation.push_back(position.x);
    observation.push_back(position.y);

    observation.push_back(body->GetAngle());

    return observation;
}
}