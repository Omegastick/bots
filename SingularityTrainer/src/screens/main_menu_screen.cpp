#include <imgui.h>

#include "screens/main_menu_screen.h"
#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
MainMenuScreen::MainMenuScreen(ResourceManager &resource_manager, Communicator &communicator, ScreenManager &screen_manager)
    : resource_manager(&resource_manager),
      communicator(&communicator),
      screen_manager(&screen_manager)
{
}

void MainMenuScreen::update(float delta_time)
{
    ImGui::Begin("Main menu :)");
    ImGui::Text("Aaaaaa");
    ImGui::End();
}

void MainMenuScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();
    renderer.end();
}
}