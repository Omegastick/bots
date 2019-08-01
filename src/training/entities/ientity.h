#pragma once

#include <memory>

#include <Box2D/Box2D.h>

#include "graphics/idrawable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
class IEntity : public IDrawable
{
  protected:
    unsigned int id;
    std::unique_ptr<RigidBody> rigid_body;

  public:
    IEntity(unsigned int id);
    virtual ~IEntity() = 0;

    virtual void destroy();
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