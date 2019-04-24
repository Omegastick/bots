#include <Box2D/Box2D.h>
#include <memory>
#include <string>
#include <vector>

#include "graphics/colors.h"
#include "training/entities/wall.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
Wall::Wall(float x, float y, float width, float height, b2World &world)
{
    // Rigidbody
    b2Vec2 position(x + (width / 2), y + (height / 2));
    rigid_body = std::make_unique<RigidBody>(b2_staticBody, position, world, this, RigidBody::ParentTypes::Wall);
    b2PolygonShape rigid_body_shape;
    rigid_body_shape.SetAsBox(width / 2, height / 2);
    b2FixtureDef fixture_def;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.shape = &rigid_body_shape;
    rigid_body->body->CreateFixture(&fixture_def);

    sprite = std::make_unique<Sprite>("pixel");
    sprite->set_scale({width, height});
    sprite->set_origin(sprite->get_center());
    sprite->set_position({x + (width / 2), y + (height / 2)});
    sprite->set_color(cl_white);
}

Wall::~Wall() {}

RenderData Wall::get_render_data(bool /*lightweight*/)
{
    RenderData render_data;
    render_data.sprites.push_back(*sprite);
    return render_data;
}

void Wall::begin_contact(RigidBody * /*other*/) {}

void Wall::end_contact(RigidBody * /*other*/) {}
}