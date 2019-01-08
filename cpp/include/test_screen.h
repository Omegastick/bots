#pragma once

#include <SFML/Graphics.hpp>

#include "iscreen.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
class TestScreen : public IScreen
{
  public:
    TestScreen(sf::RenderTarget &window, ResourceManager &resourceManager);
    ~TestScreen();

    void draw(sf::RenderTarget &render_target);
    void update(float delta_time);

  private:
    sf::Sprite arrow;
};
}