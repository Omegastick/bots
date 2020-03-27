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

namespace ai
{
void draw_line_to_point(const ImVec2 &point)
{
    auto window_size = ImGui::GetWindowSize();
    auto window_pos = ImGui::GetWindowPos();
    ImRect window_bb(window_pos + ImVec2(2, 2), window_pos + window_size - ImVec2(2, 2));
    ImVec2 origin = {std::clamp(point.x, window_bb.Min.x, window_bb.Max.x),
                     std::clamp(point.y, window_bb.Min.y, window_bb.Max.y)};
    ImGui::GetBackgroundDrawList()->AddLine(origin,
                                            point,
                                            ImGui::GetColorU32(ImGuiCol_FrameBg), 3);
}

RewardWindows::RewardWindows(IO &io)
    : io(io) {}

void RewardWindows::update(glm::mat4 &projection, RewardConfig &reward_config)
{
    auto resolution = io.get_resolutionf();

    // Hill reward
    ImGui::SetNextWindowPos({resolution.x * 0.7f, resolution.y * 0.6f}, ImGuiCond_Once);
    ImGui::Begin("Hill reward");

    ImGui::Text("Reward per 1/10th of a second spent with the hill captured");
    ImGui::InputFloat("##hill_reward",
                      &reward_config.hill_tick_reward,
                      0.1f,
                      0.5f,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);

    ImGui::Text("Reward per 1/10th of a second spent with the hill controlled by the enemy");
    ImGui::InputFloat("##hill_punishment",
                      &reward_config.enemy_hill_tick_punishment,
                      0.1f,
                      0.5f,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);

    // Draw line to edge of hill
    auto window_size = ImGui::GetWindowSize();
    auto window_pos = ImGui::GetWindowPos();
    glm::vec2 window_center = glm::vec2{window_pos.x + window_size.x * 0.5f,
                                        window_pos.y + window_size.y * 0.5f} /
                              io.get_resolutionf();
    glm::vec2 screen_center{0.5f, 0.5f};
    auto direction_vector = window_center - screen_center;
    float radius = 0.033f;
    auto line_end_point = direction_vector /
                          glm::length(direction_vector) *
                          radius *
                          static_cast<float>(io.get_resolution().x);
    line_end_point += io.get_resolution() / 2;
    draw_line_to_point({line_end_point.x, line_end_point.y});

    ImGui::End();

    // Hit enemy reward
    ImGui::SetNextWindowPos({resolution.x * 0.25f, resolution.y * 0.15f}, ImGuiCond_Once);
    ImGui::Begin("Hit enemy reward");

    ImGui::Text("Reward for hitting enemy");
    ImGui::InputFloat("##hit_enemy_reward",
                      &reward_config.hit_enemy_reward,
                      1,
                      5,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);
    if (ImGui::RadioButton("Per HP", reward_config.hit_enemy_type == HpOrHit::Hp))
    {
        reward_config.hit_enemy_type = HpOrHit::Hp;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Per hit", reward_config.hit_enemy_type == HpOrHit::Hit))
    {
        reward_config.hit_enemy_type = HpOrHit::Hit;
    }

    // Draw line to enemy
    auto enemy_position_screen = world_to_screen_space({0.f, 15.f}, resolution, projection);
    draw_line_to_point({enemy_position_screen.x, enemy_position_screen.y});

    ImGui::End();

    // Hit taken punishment
    ImGui::SetNextWindowPos({resolution.x * 0.25f, resolution.y * 0.75f}, ImGuiCond_Once);
    ImGui::Begin("Hit taken punishment");

    ImGui::Text("Punishment for being hit");
    ImGui::InputFloat("##hit_self_punishment",
                      &reward_config.hit_self_punishment,
                      1,
                      5,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);
    if (ImGui::RadioButton("Per HP", reward_config.hit_self_type == HpOrHit::Hp))
    {
        reward_config.hit_self_type = HpOrHit::Hp;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Per hit", reward_config.hit_self_type == HpOrHit::Hit))
    {
        reward_config.hit_self_type = HpOrHit::Hit;
    }

    // Draw line to body
    auto body_position_screen = world_to_screen_space({0.f, -15.f}, resolution, projection);
    draw_line_to_point({body_position_screen.x, body_position_screen.y});

    ImGui::End();

    // Victory reward
    ImGui::SetNextWindowPos({resolution.x * 0.7f, resolution.y * 0.3f}, ImGuiCond_Once);
    ImGui::Begin("Victory reward");

    ImGui::Text("Reward on win");
    ImGui::InputFloat("##victory_reward",
                      &reward_config.victory_reward,
                      10,
                      50,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);

    ImGui::Text("Punishment on loss");
    ImGui::InputFloat("##loss_punishment",
                      &reward_config.loss_punishment,
                      10,
                      50,
                      "%.1f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);

    // Draw line to edge of game area
    window_size = ImGui::GetWindowSize();
    window_pos = ImGui::GetWindowPos();
    window_center = glm::vec2{window_pos.x + window_size.x * 0.5,
                              window_pos.y + window_size.y * 0.5};
    window_center = screen_to_world_space(window_center, resolution, projection);
    line_end_point = {std::clamp(window_center.x, -10.f, 10.f),
                      std::clamp(window_center.y, -20.f, 20.f)};
    line_end_point = world_to_screen_space(line_end_point, resolution, projection);

    draw_line_to_point({line_end_point.x, line_end_point.y});

    ImGui::End();
}
}