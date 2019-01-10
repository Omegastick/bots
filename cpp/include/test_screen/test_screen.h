#pragma once

#include <SFML/Graphics.hpp>

#include "communicator.h"
#include "iscreen.h"
#include "resource_manager.h"
#include "test_env.h"

namespace SingularityTrainer
{
class TestScreen : public IScreen
{
  public:
    TestScreen(sf::RenderTarget &window, ResourceManager &resource_manager, Communicator &communicator, int env_count);
    ~TestScreen();

    void draw(sf::RenderTarget &render_target);
    void update(float delta_time);

  private:
    std::vector<std::shared_ptr<TestEnv>> environments;
};
}