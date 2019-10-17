#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "training/icollidable.h"

namespace SingularityTrainer
{
class IEnvironment;
class Sprite;
class Renderer;
class RigidBody;

class Target : public ICollidable
{
  private:
    IEnvironment &environment;
    std::unique_ptr<Sprite> sprite;

  public:
    Target(float x, float y, b2World &world, IEnvironment &env);
    ~Target();

    void draw(Renderer &renderer, bool lightweight = false);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);

    std::unique_ptr<RigidBody> rigid_body;
};
}