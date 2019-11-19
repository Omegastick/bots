#pragma once

#include <memory>
#include <mutex>

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "ientity.h"
#include "training/icollidable.h"

namespace SingularityTrainer
{
struct Sprite;
class RigidBody;
class Body;
class IEnvironment;

class Bullet : public ICollidable, public IEntity
{
  private:
    int life;
    b2Vec2 last_position;
    std::unique_ptr<Sprite> sprite;
    glm::vec4 particle_color;
    Body *owner;
    bool destroyed;

  public:
    Bullet(b2Vec2 position,
           b2Vec2 velocity,
           b2World &world,
           Body *owner,
           unsigned int id,
           IEnvironment &env);
    ~Bullet();

    virtual void begin_contact(RigidBody *other);
    virtual void destroy();
    virtual void end_contact(RigidBody *other);
    virtual void draw(Renderer &renderer, bool lightweight = false);
    virtual bool should_destroy();
    virtual void update();
};
}