#pragma once

#include "iscreen.h"

namespace SingularityTrainer
{
class Communicator;
class Random;
class Renderer;
class ResourceManager;
class ScreenManager;

class MainMenuScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    Communicator *communicator;
    ScreenManager *screen_manager;
    Random *rng;

  public:
    MainMenuScreen(ResourceManager &resource_manager, Communicator &communicator, ScreenManager &screen_manager, Random &random);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(float delta_time);
};
}