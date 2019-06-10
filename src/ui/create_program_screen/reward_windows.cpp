#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "reward_windows.h"
#include "misc/io.h"
#include "misc/utilities.h"
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
    ImGui::Begin("Hill reward");

    ImGui::Text("Reward per 1/10th of a second spent with the hill captured");
    ImGui::InputFloat("##hill_reward",
                      &reward_config.hill_tick_reward,
                      0.1,
                      0.5,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);

    ImGui::Text("Reward per 1/10th of a second spent with the hill controlled by the enemy");
    ImGui::InputFloat("##hill_punishment",
                      &reward_config.enemy_hill_tick_punishment,
                      0.1,
                      0.5,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);

    // Draw line to edge of hill
    auto window_size = ImGui::GetWindowSize();
    auto window_pos = ImGui::GetWindowPos();
    glm::vec2 window_center = glm::vec2{window_pos.x + window_size.x * 0.5, window_pos.y + window_size.y * 0.5} / static_cast<glm::vec2>(io.get_resolution());
    glm::vec2 screen_center{0.5, 0.5};
    auto direction_vector = window_center - screen_center;
    float radius = 0.033;
    auto line_end_point = direction_vector / glm::length(direction_vector) * radius * static_cast<float>(io.get_resolution().x);
    line_end_point += io.get_resolution() / 2;
    draw_line_to_point({line_end_point.x, line_end_point.y});

    ImGui::End();
}
}