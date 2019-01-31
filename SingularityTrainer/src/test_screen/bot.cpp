#include "test_screen/bot.h"
#include "training/rigid_body.h"
#include "gui/colors.h"
#include "utilities.h"

namespace SingularityTrainer
{
Bot::Bot(ResourceManager &resource_manager, b2World &world)
{
    // Sprite
    sprite.setTexture(*resource_manager.texture_store.get("arrow"));
    sprite.setScale(0.0025, 0.0025);
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
    sprite.setColor(cl_white);

    // Rigid body
    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, b2Vec2_zero, world, this, RigidBody::ParentTypes::Bot);
    b2Vec2 vertices[3];
    vertices[0].Set(0, -0.7);
    vertices[1].Set(0.49, 0.74);
    vertices[2].Set(-0.49, 0.74);
    b2PolygonShape shape;
    shape.Set(vertices, 3);
    b2FixtureDef fixture_def;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.shape = &shape;
    rigid_body->body->CreateFixture(&fixture_def);
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

void Bot::act(std::vector<int> &actions)
{
    float speed = 60;
    if (actions[0])
    {
        float angle = rigid_body->body->GetAngle();
        b2Vec2 force(std::sin(angle) * speed, std::cos(angle) * speed);
        rigid_body->body->ApplyForceToCenter(force, true);
    }
    if (actions[1])
    {
        float angle = rigid_body->body->GetAngle();
        b2Vec2 force(-std::sin(angle) * speed, -std::cos(angle) * speed);
        rigid_body->body->ApplyForceToCenter(force, true);
    }
    if (actions[2])
    {
        rigid_body->body->ApplyTorque(0.1 * speed, true);
    }
    if (actions[3])
    {
        rigid_body->body->ApplyTorque(-0.1 * speed, true);
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
