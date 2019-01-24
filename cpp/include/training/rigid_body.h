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
        Agent,
        Wall,
        Target,
        Bot
    };

    RigidBody(b2BodyType type, b2Vec2 position, b2World &world, void *parent, ParentTypes parent_type);
    ~RigidBody();

    void *parent;
    ParentTypes parent_type;
    b2Body *body;
    b2BodyDef body_def;
};
}