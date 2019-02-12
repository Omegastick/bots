#pragma once

#include <Thor/Input.hpp>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "input.h"
#include "graphics/renderer.h"

namespace SingularityTrainer
{
class IScreen : IDrawable
{
  public:
    IScreen(){};
    ~IScreen(){};

    virtual void update(const float delta_time) = 0;
    virtual void draw(const float delta_time, const Renderer &renderer, bool lightweight = false) = 0;
};
}