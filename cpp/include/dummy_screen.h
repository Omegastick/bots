#pragma once

#include <SFML/Graphics.hpp>

#include "iscreen.h"
#include "resource_manager.h"
#include "communicator.h"

namespace SingularityTrainer
{
class DummyScreen : public IScreen
{
  public:
    DummyScreen(sf::RenderTarget &window, ResourceManager &resource_manager, Communicator &communicator);
    ~DummyScreen();

    void draw(sf::RenderTarget &render_target);
    void update(float delta_time);

  private:
    sf::Sprite arrow;
};
}