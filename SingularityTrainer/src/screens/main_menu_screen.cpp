#include <imgui.h>
#include <spdlog/spdlog.h>

#include "screens/main_menu_screen.h"
#include "screens/koth_env_screen.h"
#include "screens/watch_screen.h"
#include "graphics/renderers/renderer.h"
#include "screen_manager.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
MainMenuScreen::MainMenuScreen(ResourceManager &resource_manager, Communicator &communicator, ScreenManager &screen_manager, Random &rng)
    : resource_manager(&resource_manager),
      communicator(&communicator),
      screen_manager(&screen_manager),
      rng(&rng)
{
}

void MainMenuScreen::update(float delta_time)
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1, 1, 1, 0.1});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {1, 1, 1, 0.05});
    ImGui::PushStyleColor(ImGuiCol_Text, {cl_base3.r, cl_base3.g, cl_base3.b, 1});
    auto io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[1]);

    ImGui::SetNextWindowPosCenter();
    ImGui::Begin("Main menu :)", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    if (ImGui::Button("Train Agent"))
    {
        screen_manager->show_screen(std::make_shared<KothEnvScreen>(*resource_manager, *communicator, *rng, 7));
    }
    if (ImGui::Button("Load Agent"))
    {
        screen_manager->show_screen(std::make_shared<WatchScreen>(*resource_manager, *communicator, *rng));
    }
    ImGui::End();
    ImGui::PopFont();
    ImGui::PopStyleColor(5);

    ImGui::ShowStyleEditor();
}

void MainMenuScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();
    renderer.end();
}
}