#pragma once

#include "iscreen.h"

namespace SingularityTrainer
{
class ResourceManager;
class Communicator;
class Renderer;
class ScreenManager;

class MainMenuScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    Communicator *communicator;
    ScreenManager *screen_manager;

  public:
    MainMenuScreen(ResourceManager &resource_manager, Communicator &communicator, ScreenManager &screen_manager);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(float delta_time);
};
}