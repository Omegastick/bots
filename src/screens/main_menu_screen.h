#pragma once

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class IO;
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
    IO *io;

  public:
    MainMenuScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, Random &random, IO &io);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class MainMenuScreenFactory : public IScreenFactory
{
  private:
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    IO &io;
    Random &rng;

  public:
    MainMenuScreenFactory(ResourceManager &resource_manager, ScreenManager &screen_manager, IO &io, Random &rng)
        : resource_manager(resource_manager), screen_manager(screen_manager), io(io), rng(rng) {}

    inline std::shared_ptr<IScreen> make()
    {
        return std::make_shared<MainMenuScreen>(resource_manager, screen_manager, rng, io);
    }
};
}