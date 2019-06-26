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
#include "screens/create_program_screen.h"
#include "training/training_program.h"
#include "misc/screen_manager.h"

namespace SingularityTrainer
{
MainMenuScreen::MainMenuScreen(ScreenManager &screen_manager,
                               BuildScreenFactory &build_screen_factory,
                               CreateProgramScreenFactory &create_program_screen_factory)
    : screen_manager(screen_manager),
      build_screen_factory(build_screen_factory),
      create_program_screen_factory(create_program_screen_factory) {}

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
        screen_manager.show_screen(create_program_screen_factory.make());
    }
    if (ImGui::Button("Build Body"))
    {
        screen_manager.show_screen(build_screen_factory.make());
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