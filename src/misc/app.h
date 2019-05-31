#pragma once

#include <argh.h>

namespace SingularityTrainer
{
class IO;
class MainMenuScreenFactory;
class Renderer;
class ScreenManager;
class Window;

class App
{
  private:
    IO &io;
    Renderer &renderer;
    ScreenManager &screen_manager;
    double time;
    Window &window;

  public:
    App(IO &io, Renderer &renderer, MainMenuScreenFactory &main_menu_screen_factory, ScreenManager &screen_manager, Window &window);

    int run(int argc, char *argv[]);
    int run_tests(int argc, char *argv[], const argh::parser &args);
};
}
