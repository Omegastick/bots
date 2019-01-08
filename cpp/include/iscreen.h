#pragma once

#include <SFML/Graphics.hpp>

namespace SingularityTrainer
{
class IScreen
{
  public:
    IScreen(){};
    ~IScreen(){};

    virtual void draw(sf::RenderTarget &render_target) = 0;
    virtual void update(float delta_time) = 0;
};
}