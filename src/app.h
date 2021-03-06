#pragma once

#include <memory>

#include <argh.h>

#include "graphics/post_processing/post_proc_layer.h"

namespace ai
{
class Animator;
class AudioEngine;
class Background;
class IO;
class MainMenuScreenFactory;
class Renderer;
class ResourceManager;
class ScreenManager;
class Window;

class App
{
  private:
    Animator &animator;
    AudioEngine &audio_engine;
    Background &background;
    std::unique_ptr<PostProcLayer> bloom_post_proc_layer;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    std::unique_ptr<PostProcLayer> tone_map_post_proc_layer;
    IO &io;
    MainMenuScreenFactory &main_menu_screen_factory;
    Renderer &renderer;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    double time;
    Window &window;

    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    App(Animator &animator,
        AudioEngine &audio_engine,
        Background &background,
        IO &io,
        Renderer &renderer,
        ResourceManager &resource_manager,
        MainMenuScreenFactory &main_menu_screen_factory,
        ScreenManager &screen_manager,
        Window &window);

    int run(int argc, char *argv[]);
};
}
