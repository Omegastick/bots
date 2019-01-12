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
    sprite.setScale(0.0025, 0.0025);
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

    // Rigid body
    b2Vec2 vertices[3];
    vertices[0].Set(0, -0.7);
    vertices[1].Set(0.49, 0.74);
    vertices[2].Set(-0.49, 0.74);
    b2PolygonShape shape;
    shape.Set(vertices, 3);
    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, b2Vec2_zero, world, shape, this, RigidBody::ParentTypes::Bot);
}

Bot::~Bot() {}

void Bot::draw(sf::RenderTarget &render_target)
{
    b2Vec2 world_position = rigid_body->body->GetPosition();
    sf::Vector2f screen_position(world_position.x, world_position.y);
    sprite.setPosition(screen_position);
    sprite.setRotation(rad_to_deg(rigid_body->body->GetAngle()));
    render_target.draw(sprite);
}

void Bot::act(std::vector<bool> actions)
{
    if (actions[0])
    {
        float angle = rigid_body->body->GetAngle();
        b2Vec2 force(std::sin(angle) * 0.1, std::cos(angle) * 0.1);
        rigid_body->body->ApplyForceToCenter(force, true);
    }
    if (actions[1])
    {
        float angle = rigid_body->body->GetAngle();
        b2Vec2 force(std::sin(angle) * -0.1, std::cos(angle) * -0.1);
        rigid_body->body->ApplyForceToCenter(force, true);
    }
    if (actions[2])
    {
        rigid_body->body->ApplyTorque(0.01, true);
    }
    if (actions[3])
    {
        rigid_body->body->ApplyTorque(-0.01, true);
    }
}

std::vector<float> Bot::get_observation()
{
    std::vector<float> observation;

    b2Vec2 position = rigid_body->body->GetPosition();
    observation.push_back(position.x);
    observation.push_back(position.y);

    observation.push_back(rigid_body->body->GetAngle());

    return observation;
}

void Bot::begin_contact(RigidBody *other) {}
void Bot::end_contact(RigidBody *other) {}
}
