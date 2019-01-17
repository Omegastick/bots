#pragma once

#include <SFML/Graphics.hpp>

#include "communicator.h"
#include "iscreen.h"
#include "requests.h"
#include "resource_manager.h"
#include "test_screen/test_env.h"
#include "gui/gui_panel.h"

namespace SingularityTrainer
{
class TestScreen : public IScreen
{
  public:
    TestScreen(std::shared_ptr<ResourceManager> resource_manager, std::shared_ptr<Communicator> communicator, int env_count);
    ~TestScreen();

    void draw(sf::RenderTarget &render_target);
    void update(const sf::Time &delta_time, sf::RenderWindow &window);

  private:
    std::shared_ptr<Communicator> communicator;
    std::vector<std::unique_ptr<TestEnv>> environments;
    std::vector<std::vector<float>> observations;
    int frame_counter;
    int action_frame_counter;
    std::atomic<bool> waiting_for_server;
    std::future<std::unique_ptr<Response<GiveRewardsResult>>> model_update_finished;
    GUIPanel panel;

    void fast_update();
    void slow_update(bool action_frame);
    void action_update();
};
}