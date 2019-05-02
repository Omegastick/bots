#include <memory>

#include <imgui.h>

#include "graphics/renderers/renderer.h"
#include "screens/build_screen.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "ui/build_screen/part_selector_window.h"

namespace SingularityTrainer
{
BuildScreen::BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager)
    : resource_manager(&resource_manager),
      screen_manager(&screen_manager),
      part_selector_window(std::make_unique<PartSelectorWindow>())
{
}

void BuildScreen::update(double /*delta_time*/)
{
    part_selector_window->update();
}

void BuildScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();
    renderer.end();
}
}