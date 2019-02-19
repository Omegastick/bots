#pragma once

#include <vector>

#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/element_buffer.h"

namespace SingularityTrainer
{
class IParticleSystem
{
  private:
    std::unique_ptr<VertexBuffer> quad_vertex_buffer;

  public:
    IParticleSystem(){};
    virtual ~IParticleSystem() = 0;

    virtual void update(float delta_time) = 0;
    virtual bool full() = 0;
};

inline IParticleSystem::~IParticleSystem() {}
}