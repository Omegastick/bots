#pragma once

#include <SFML/Graphics.hpp>

#include "iscreen.h"

namespace SingularityTrainer
{
class TestScreen : public IScreen
{
  public:
    TestScreen(sf::RenderTarget &window);
    ~TestScreen();

    void draw(sf::RenderTarget &render_target);
    void update(float delta_time);

  private:
    sf::RectangleShape square;
};
}