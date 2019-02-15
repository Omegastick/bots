#pragma once

#include <vector>

#include "graphics/vertex_buffer.h"
#include "graphics/vertex_array.h"
#include "graphics/element_buffer.h"

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