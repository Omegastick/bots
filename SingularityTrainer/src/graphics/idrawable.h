#pragma once

#include "graphics/render_data.h"

namespace SingularityTrainer
{
class IDrawable
{
  public:
    virtual ~IDrawable() = 0;

    virtual RenderData get_render_data(bool lightweight = false) = 0;
};

inline IDrawable::~IDrawable() {}
}