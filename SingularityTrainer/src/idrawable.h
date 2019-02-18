#pragma once

#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
class IDrawable
{
  public:
    IDrawable(){};
    ~IDrawable(){};

    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
};
}