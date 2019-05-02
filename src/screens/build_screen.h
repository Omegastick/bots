#pragma once

#include <memory>

#include "iscreen.h"
#include "ui/build_screen/part_selector_window.h"

namespace SingularityTrainer
{
class Renderer;
class ResourceManager;
class ScreenManager;

class BuildScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    ScreenManager *screen_manager;
    std::unique_ptr<PartSelectorWindow> part_selector_window;

  public:
    BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};
}