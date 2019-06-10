#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

#include "reward_windows.h"
#include "misc/io.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
void draw_line_to_point(const ImVec2 &point)
{
    auto window_size = ImGui::GetWindowSize();
    auto window_pos = ImGui::GetWindowPos();
    ImRect window_bb(window_pos + ImVec2(2, 2), window_pos + window_size - ImVec2(2, 2));
    ImVec2 origin = {std::clamp(point.x, window_bb.Min.x, window_bb.Max.x),
                     std::clamp(point.y, window_bb.Min.y, window_bb.Max.y)};
    ImGui::GetBackgroundDrawList()->AddLine(origin, point, ImGui::GetColorU32(ImGuiCol_FrameBg), 3);
}

RewardWindows::RewardWindows(IO &io)
    : io(io) {}

void RewardWindows::update(RewardConfig &reward_config)
{
    ImGui::Begin("Line test");

    ImGui::Text("Hello there, I am bob");
    auto resolution = io.get_resolution();
    draw_line_to_point({resolution.x / 2., resolution.y / 2.});

    ImGui::End();
}
}