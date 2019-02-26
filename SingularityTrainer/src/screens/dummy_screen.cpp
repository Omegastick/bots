#include "communicator.h"
#include "requests.h"
#include "resource_manager.h"
#include "dummy_screen.h"

namespace SingularityTrainer
{
DummyScreen::DummyScreen(std::shared_ptr<ResourceManager> resource_manager, std::shared_ptr<Communicator> communicator)
{
}

void DummyScreen::draw(Renderer &renderer, bool lightweight)
{
}
void DummyScreen::update(float delta_time)
{
}
}