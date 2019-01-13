#pragma once

#include <SFML/Graphics.hpp>

#include "communicator.h"
#include "iscreen.h"
#include "resource_manager.h"
#include "requests.h"
#include "test_screen/test_env.h"

namespace SingularityTrainer
{
class TestScreen : public IScreen
{
  public:
    TestScreen(std::shared_ptr<ResourceManager> resource_manager, std::shared_ptr<Communicator> communicator, int env_count);
    ~TestScreen();

    void draw(sf::RenderTarget &render_target);
    void update(float delta_time);

  private:
    std::shared_ptr<Communicator> communicator;
    std::vector<std::unique_ptr<TestEnv>> environments;
    std::vector<std::vector<float>> observations;
};
}