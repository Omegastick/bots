#include <memory>

#include <Box2D/Box2D.h>
#include <spdlog/spdlog.h>

#include "training/entities/hill.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "training/bodies/body.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
Hill::Hill(float x, float y, b2World &world)
{
    // Rigid body
    rigid_body = std::make_unique<RigidBody>(b2_staticBody, b2Vec2(x, y), world, this, RigidBody::ParentTypes::Hill);
    b2CircleShape rigid_body_shape;
    rigid_body_shape.m_radius = 3;
    b2FixtureDef fixture_def;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.shape = &rigid_body_shape;
    fixture_def.isSensor = true;
    rigid_body->body->CreateFixture(&fixture_def);

    // Sprite
    sprite = std::make_unique<Sprite>();
    sprite->texture = "target";
    sprite->transform.set_scale({6, 6});
}

void Hill::draw(Renderer &renderer, bool /*lightweight*/)
{
    b2Vec2 position = rigid_body->body->GetPosition();
    sprite->transform.set_position({position.x, position.y});
    renderer.draw(*sprite);
}

void Hill::begin_contact(RigidBody *other)
{
    if (other->parent_type == RigidBody::ParentTypes::Body)
    {
        occupants[static_cast<Body *>(other->parent)]++;
    }
}

void Hill::end_contact(RigidBody *other)
{
    if (other->parent_type == RigidBody::ParentTypes::Body)
    {
        auto other_body = static_cast<Body *>(other->parent);
        occupants[other_body]--;
        if (occupants[other_body] == 0)
        {
            occupants.erase(other_body);
        }
    }
}

void Hill::register_callback(std::function<void(const std::unordered_map<Body *, int> &)> callback)
{
    this->callback = callback;
}

void Hill::update()
{
    if (callback)
    {
        // Remove agents that have left the hill, but haven't been removed from the list for some
        // reason
        for (auto it = occupants.begin(); it != occupants.end();)
        {
            if (it->second == 0)
            {
                it = occupants.erase(it);
            }
            else
            {
                it++;
            }
        }
        callback(occupants);
    }
}

void Hill::reset()
{
    for (const auto &occupant : occupants)
    {
        occupants[occupant.first] = 0;
    }
}
}