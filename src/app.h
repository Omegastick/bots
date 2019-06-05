#pragma once

#include <argh.h>

namespace SingularityTrainer
{
class Animator;
class IO;
class MainMenuScreenFactory;
class Renderer;
class ScreenManager;
class Window;

class App
{
  private:
  Animator &animator;
    IO &io;
    MainMenuScreenFactory &main_menu_screen_factory;
    Renderer &renderer;
    ScreenManager &screen_manager;
    double time;
    Window &window;

    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    App(Animator &animator, IO &io, Renderer &renderer, MainMenuScreenFactory &main_menu_screen_factory, ScreenManager &screen_manager, Window &window);

    int run(int argc, char *argv[]);
};
}
