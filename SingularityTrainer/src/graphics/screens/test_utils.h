#pragma once

#include <string>
#include <vector>
#include <memory>

#include <imgui.h>

#include "iscreen.h"
#include "screen_manager.h"
#include "graphics/imgui_utils.h"

namespace SingularityTrainer
{
inline void display_test_dialog(
    const std::string &test_title,
    std::vector<std::shared_ptr<IScreen>> &screens,
    std::vector<std::string> &screen_names,
    ScreenManager &screen_manager)
{
    ImGui::Begin("Screens", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    int own_index = std::find(screen_names.begin(), screen_names.end(), test_title) - screen_names.begin();
    int screen_index = own_index;
    ImGui::Combo("", &screen_index, screen_names);
    if (screen_index != own_index)
    {
        screen_manager.close_screen();
        screen_manager.show_screen(screens[screen_index]);
    }
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}
}