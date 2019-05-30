#pragma once

#include <string>
#include <vector>

#include <Box2D/Box2D.h>

namespace SingularityTrainer
{
class RigidBody
{
  public:
    enum ParentTypes
    {
        Agent,
        Wall,
        Target,
        Bullet,
        Bot,
        Hill
    };

    RigidBody(b2BodyType type, b2Vec2 position, b2World &world, void *parent, ParentTypes parent_type);
    ~RigidBody();

    void destroy();

    void *parent;
    ParentTypes parent_type;
    b2Body *body;
    b2BodyDef body_def;
    b2World *world;
};
}