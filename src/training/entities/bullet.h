#pragma once

#include <memory>
#include <mutex>

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "graphics/idrawable.h"
#include "training/icollidable.h"

namespace SingularityTrainer
{
class RenderData;
class Sprite;
class RigidBody;
class Agent;
class IEnvironment;

class Bullet : public IDrawable, public ICollidable
{
  private:
    int life;
    b2Vec2 last_position;
    std::unique_ptr<Sprite> sprite;
    std::vector<Particle> explosion_particles;
    glm::vec4 particle_color;
    Agent *owner;
    std::mutex particle_mutex;

  public:
    Bullet(b2Vec2 position, b2Vec2 velocity, b2World &world, Agent *owner);
    ~Bullet();

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    void update();

    bool destroyed;
    std::unique_ptr<RigidBody> rigid_body;
};
}