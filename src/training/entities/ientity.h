#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "training/rigid_body.h"

namespace ai
{
class IEnvironment;
class Renderer;

class IEntity
{
  protected:
    unsigned int id;
    IEnvironment &env;
    std::unique_ptr<RigidBody> rigid_body;

  public:
    IEntity(unsigned int id, IEnvironment &env);
    virtual ~IEntity() = 0;

    virtual void destroy();
    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
    virtual b2Transform get_transform() const;
    virtual float get_angular_velocity() const;
    virtual b2Vec2 get_linear_velocity() const;
    virtual void set_transform(b2Vec2 position, float rotation);
    virtual void set_angular_velocity(float angular_velocity);
    virtual void set_linear_velocity(b2Vec2 linear_velocity);
    virtual bool should_destroy() = 0;
    virtual void update() = 0;

    inline unsigned int get_id() const { return id; }
};

inline IEntity::~IEntity() {}
}