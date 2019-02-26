#pragma once

#include <memory>

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "graphics/render_data.h"
#include "graphics/sprite.h"
#include "graphics/idrawable.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class Bullet : public IDrawable, public ICollidable
{
  private:
    b2Vec2 last_position;
    std::unique_ptr<Sprite> sprite;
    std::vector<Particle> explosion_particles;
    glm::vec4 particle_color;

  public:
    Bullet(b2Vec2 position, b2Vec2 velocity, b2World &world);
    ~Bullet();

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);
    void update();

    bool destroyed;
    int life;
    std::unique_ptr<RigidBody> rigid_body;
};
}