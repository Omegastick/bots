#include <imgui.h>
#include <glm/vec2.hpp>

#include "back_button.h"
#include "misc/screen_manager.h"

namespace ai
{
void back_button(ScreenManager &screen_manager, const glm::vec2 &resolution)
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {12, 6});
    auto imgui_io = ImGui::GetIO();
    ImGui::PushFont(imgui_io.Fonts->Fonts[2]);
    ImGui::SetNextWindowPos({0, resolution.y}, ImGuiCond_Always, {0, 1});
    ImGui::Begin("##back", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    if (ImGui::Button("Back"))
    {
        screen_manager.close_screen();
    }
    ImGui::End();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
}