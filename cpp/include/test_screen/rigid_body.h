#pragma once

#include <Box2D/Box2D.h>
#include <string>
#include <vector>

namespace SingularityTrainer
{
class RigidBody
{
  public:
    std::vector<std::string> labels;

  protected:
    b2Body *body;
    b2BodyDef body_def;
    b2PolygonShape polygon_shape;
    b2FixtureDef fixture_def;
};
}