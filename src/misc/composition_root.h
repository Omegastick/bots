#pragma once

#include <memory>

#include "graphics/renderers/renderer.h"
#include "graphics/window.h"
#include "misc/app.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/main_menu_screen.h"

namespace SingularityTrainer
{
class CompositionRoot
{
  private:
    int resolution_x, resolution_y;

    std::unique_ptr<IO> io;
    std::unique_ptr<ResourceManager> resource_manager;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<ScreenManager> screen_manager;
    std::unique_ptr<Window> window;
    std::unique_ptr<Random> rng;

    std::unique_ptr<MainMenuScreenFactory> main_menu_screen_factory;

  public:
    CompositionRoot(int resolution_x, int resolution_y);

    std::unique_ptr<App> make_app();
};
}