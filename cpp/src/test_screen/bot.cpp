#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <math.h>
#include <memory>

#include "test_screen/bot.h"

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
    body_def.position.Set(1, 0);
    body = world.CreateBody(&body_def);
    b2Vec2 vertices[3];
    vertices[0].Set(0, -0.375);
    vertices[1].Set(0.25, 0.375);
    vertices[2].Set(-0.25, 0.375);
    polygon_shape.Set(vertices, 3);
    fixture_def.shape = &polygon_shape;
    fixture_def.density = 1.0;
    fixture_def.friction = 1.0;
    body->CreateFixture(&fixture_def);
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
        body->ApplyForceToCenter(b2Vec2(0, 0.1), true);
    }
    if (actions[1])
    {
        body->ApplyForceToCenter(b2Vec2(0, -0.1), true);
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
}