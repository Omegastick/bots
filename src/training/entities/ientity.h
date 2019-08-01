#pragma once

#include "graphics/idrawable.h"

namespace SingularityTrainer
{
class IEntity : public IDrawable
{
  protected:
    unsigned int id;

  public:
    inline IEntity(unsigned int id) : id(id) {}
    virtual ~IEntity() = 0;

    virtual void destroy() = 0;
    virtual bool should_destroy() = 0;
    virtual void update() = 0;
};

inline IEntity::~IEntity() {}
}