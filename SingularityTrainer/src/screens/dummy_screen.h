#pragma once

#include <memory>

#include "iscreen.h"
#include "resource_manager.h"
#include "communicator.h"
#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
class DummyScreen : public IScreen
{
  public:
    DummyScreen(std::shared_ptr<ResourceManager> resource_manager, std::shared_ptr<Communicator> communicator);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(float delta_time);
};
}