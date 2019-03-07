#include <Box2D/Box2D.h>
#include <memory>

#include "training/entities/hill.h"
#include "graphics/colors.h"
#include "graphics/idrawable.h"
#include "training/environments/ienvironment.h"
#include "training/agents/iagent.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

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

RenderData Hill::get_render_data(bool lightweight)
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
        if (std::find(occupants.begin(), occupants.end(), static_cast<IAgent *>(other->parent)) == occupants.end())
        {
            occupants.push_back(static_cast<IAgent *>(other->parent));
        }
    }
}

void Hill::end_contact(RigidBody *other)
{
    auto occupant_index = std::find(occupants.begin(), occupants.end(), static_cast<IAgent *>(other->parent));
    if (occupant_index != occupants.end())
    {
        occupants.erase(occupant_index);
    }
}
}