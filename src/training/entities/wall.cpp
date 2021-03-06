#include <Box2D/Box2D.h>
#include <memory>
#include <string>
#include <vector>

#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "training/entities/wall.h"
#include "training/rigid_body.h"

namespace ai
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

    sprite = std::make_unique<Sprite>();
    sprite->texture = "pixel";
    sprite->transform.set_scale({width, height});
    sprite->transform.set_position({x + (width / 2), y + (height / 2)});
    sprite->color = cl_white;
}

void Wall::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.draw(*sprite);
}

void Wall::begin_contact(RigidBody * /*other*/) {}

void Wall::end_contact(RigidBody * /*other*/) {}
}