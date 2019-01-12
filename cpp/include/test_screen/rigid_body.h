#pragma once

#include <Box2D/Box2D.h>
#include <string>
#include <vector>

namespace SingularityTrainer
{
class RigidBody
{
  public:
    enum ParentTypes
    {
        Bot,
        Wall,
        Target
    };
    void *parent;
    ParentTypes parent_type;
    b2Body *body;
    b2BodyDef body_def;
    b2PolygonShape polygon_shape;
    b2FixtureDef fixture_def;

    RigidBody();
    RigidBody(b2BodyType type, b2Vec2 position, b2World &world, b2Shape &shape, void *parent, ParentTypes parent_type);
    ~RigidBody();
};
}