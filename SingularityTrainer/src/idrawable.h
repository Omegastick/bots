#pragma once

#include "graphics/renderer.h"

namespace SingularityTrainer
{
class IDrawable
{
  public:
    IDrawable(){};
    ~IDrawable(){};

    virtual void draw(bool lightweight = false) = 0;
};
}