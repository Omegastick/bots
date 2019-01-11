#include "test_screen/test_screen.h"
#include "communicator.h"
#include "resource_manager.h"
#include "test_screen/test_env.h"

namespace SingularityTrainer
{
TestScreen::TestScreen(std::shared_ptr<ResourceManager> resource_manager, std::shared_ptr<Communicator> communicator, int env_count)
{
    this->communicator = communicator;
    resource_manager->load_texture("arrow", "cpp/assets/images/Arrow.png");
    environments.push_back(std::make_unique<TestEnv>(resource_manager, 460, 40, 1));
}

TestScreen::~TestScreen(){};

void TestScreen::update(float delta_time)
{
    for (auto &environment : environments)
    {
        std::vector<bool> actions{true, false, true, false};
        environment->step(actions);
    }
}

void TestScreen::draw(sf::RenderTarget &render_target)
{
    for (auto &environment : environments)
    {
        environment->draw(render_target);
    }
}
}