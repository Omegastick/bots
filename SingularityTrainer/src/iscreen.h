#pragma once

namespace SingularityTrainer
{
class Renderer;

class IScreen
{
  public:
    IScreen(){};
    ~IScreen(){};

    virtual void update(const float delta_time) = 0;
    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
};
}