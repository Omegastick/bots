#pragma once

#include <SFML/Graphics.hpp>

#include "iscreen.h"
#include "resource_manager.h"
#include "communicator.h"

namespace SingularityTrainer
{
class TestScreen : public IScreen
{
  public:
    TestScreen(sf::RenderTarget &window, ResourceManager &resource_manager, Communicator &communicator);
    ~TestScreen();

    void draw(sf::RenderTarget &render_target);
    void update(float delta_time);

  private:
    sf::Sprite arrow;
};
}