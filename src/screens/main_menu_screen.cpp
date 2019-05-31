#include <Box2D/Box2D.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "screens/main_menu_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
#include "misc/io.h"
#include "misc/resource_manager.h"
#include "screens/iscreen.h"
#include "screens/build_screen.h"
#include "screens/training_wizard_screen.h"
#include "screens/watch_screen.h"
#include "misc/screen_manager.h"

namespace SingularityTrainer
{
MainMenuScreen::MainMenuScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, Random &rng, IO &io)
    : resource_manager(&resource_manager),
      screen_manager(&screen_manager),
      rng(&rng),
      io(&io)
{
}

void MainMenuScreen::update(double /*delta_time*/)
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1, 1, 1, 0.1});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {1, 1, 1, 0.05});
    ImGui::PushStyleColor(ImGuiCol_Text, {cl_base3.r, cl_base3.g, cl_base3.b, 1});
    auto imgui_io = ImGui::GetIO();
    ImGui::PushFont(imgui_io.Fonts->Fonts[2]);

    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    ImGui::Begin("Main menu :)", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    if (ImGui::Button("Train Agent"))
    {
        screen_manager->show_screen(std::make_shared<TrainingWizardScreen>(*resource_manager, *screen_manager, *rng, *io));
    }
    if (ImGui::Button("Load Agent"))
    {
        screen_manager->show_screen(std::make_shared<WatchScreen>(*resource_manager, *io));
    }
    if (ImGui::Button("Build Body"))
    {
        screen_manager->show_screen(std::make_shared<BuildScreen>(*resource_manager, *screen_manager, *io, *rng));
    }
    ImGui::End();
    ImGui::PopFont();
    ImGui::PopStyleColor(5);

    // ImGui::ShowStyleEditor();
    // ImGui::ShowDemoWindow();
}

void MainMenuScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();
    renderer.end();
}
}