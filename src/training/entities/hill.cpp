#include <memory>

#include <Box2D/Box2D.h>
#include <spdlog/spdlog.h>

#include "training/entities/hill.h"
#include "graphics/colors.h"
#include "graphics/idrawable.h"
#include "training/environments/ienvironment.h"
#include "training/agents/agent.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
Hill::Hill(float x, float y, b2World &world, IEnvironment &env) : environment(env)
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
    sprite = std::make_unique<Sprite>("target");
    sprite->set_scale({6, 6});
    sprite->set_origin(sprite->get_center());
}

Hill::~Hill() {}

RenderData Hill::get_render_data(bool /*lightweight*/)
{
    auto render_data = RenderData();

    b2Vec2 position = rigid_body->body->GetPosition();
    sprite->set_position({position.x, position.y});
    render_data.sprites.push_back(*sprite);

    return render_data;
}

void Hill::begin_contact(RigidBody *other)
{
    if (other->parent_type == RigidBody::ParentTypes::Agent)
    {
        occupants[static_cast<Agent *>(other->parent)]++;
    }
}

void Hill::end_contact(RigidBody *other)
{
    if (other->parent_type == RigidBody::ParentTypes::Agent)
    {
        occupants[static_cast<Agent *>(other->parent)]--;
    }
}

void Hill::update() const
{
    // First check if only one person is occupying the hill
    int total_occupants = 0;
    for (const auto &agent : occupants)
    {
        if (agent.second > 0)
        {
            total_occupants++;
        }
    }

    // If so, apply the appropriate rewards to the agents
    if (total_occupants == 1)
    {
        auto &reward_config = environment.get_reward_config();
        for (auto agent : occupants)
        {
            if (agent.second > 0)
            {
                environment.change_reward(agent.first, reward_config.hill_tick_reward);
            }
            else
            {
                environment.change_reward(agent.first, reward_config.enemy_hill_tick_punishment);
            }
        }
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