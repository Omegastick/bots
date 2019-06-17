#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

#include "tabs.h"
#include "graphics/colors.h"
#include "misc/io.h"
#include "ui/create_program_screen/create_program_screen_state.h"

namespace ImGui
{
bool TabButton(const char *label, bool active)
{
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = {label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f};

    const ImRect bb(pos, pos + size);
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed)
        MarkItemEdited(id);

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    if (active)
    {
        window->DrawList->AddLine({bb.Min.x, bb.Max.y}, {bb.Max.x, bb.Max.y}, GetColorU32(ImGuiCol_Text));
    }

    if (g.LogEnabled)
        LogRenderedText(&bb.Min, active ? "(x)" : "( )");

    return pressed;
}

bool TabButton(const char *label, int *v, int v_button)
{
    const bool pressed = TabButton(label, *v == v_button);
    if (pressed)
        *v = v_button;
    return pressed;
}
}

namespace SingularityTrainer
{
Tabs::Tabs(IO &io) : io(io), selected_tab(CreateProgramScreenState::Body) {}

CreateProgramScreenState Tabs::update()
{
    auto resolution = io.get_resolution();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1, 1, 1, 0.1});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {1, 1, 1, 0});
    ImGui::PushStyleColor(ImGuiCol_Text, {cl_base3.r, cl_base3.g, cl_base3.b, 1});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {resolution.x * 0.03f, resolution.y * 0.02f});
    auto imgui_io = ImGui::GetIO();
    ImGui::PushFont(imgui_io.Fonts->Fonts[2]);

    ImGui::SetNextWindowPos({resolution.x * 0.5f, resolution.y * 0.01f}, ImGuiCond_Always, {0.5, 0.0});
    ImGui::Begin("Main menu :)", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    int selected_number = static_cast<int>(selected_tab);
    ImGui::TabButton("Body", &selected_number, CreateProgramScreenState::Body);
    ImGui::SameLine();
    ImGui::TabButton("Algorithm", &selected_number, CreateProgramScreenState::Algorithm);
    ImGui::SameLine();
    ImGui::TabButton("Rewards", &selected_number, CreateProgramScreenState::Rewards);
    ImGui::SameLine();
    ImGui::TabButton("Brain", &selected_number, CreateProgramScreenState::Brain);
    ImGui::SameLine();
    ImGui::TabButton("Save/Load", &selected_number, CreateProgramScreenState::SaveLoad);
    ImGui::End();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(5);
    selected_tab = static_cast<CreateProgramScreenState>(selected_number);

    return selected_tab;
}
}