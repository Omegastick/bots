#pragma once

#include "iscreen.h"

namespace SingularityTrainer
{
class Random;
class Renderer;
class ResourceManager;
class ScreenManager;

class MainMenuScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    ScreenManager *screen_manager;
    Random *rng;

  public:
    MainMenuScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, Random &random);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};
}