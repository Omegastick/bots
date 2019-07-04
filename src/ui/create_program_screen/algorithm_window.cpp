#include <imgui.h>

#include "algorithm_window.h"
#include "misc/io.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
void help_marker(const std::string_view text)
{
    ImGui::TextDisabled("[?]");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(std::string(text).c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

AlgorithmWindow::AlgorithmWindow(IO &io)
    : io(io) {}

void AlgorithmWindow::update(HyperParameters &hyperparams)
{
    auto resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.2f}, ImGuiCond_Once);
    ImGui::Begin("Select an algorithm");

    const float label_spacing = resolution.x * 0.08;

    const char *algorithms[] = {"A2C", "PPO"};
    auto selected_algorithm = static_cast<int>(hyperparams.algorithm);
    ImGui::Text("Algorithm:");
    ImGui::SameLine(label_spacing);
    ImGui::Combo("##algorithm", &selected_algorithm, algorithms, 2);
    hyperparams.algorithm = static_cast<Algorithm>(selected_algorithm);
    ImGui::SameLine();
    help_marker(R"(The training algorithm to use.
A2C: Simple, fast, but potentially unstable. Very sample inneficient.
PPO: More complex than A2C, but more stable and more sample efficient.)");

    ImGui::Text("Batch size:");
    ImGui::SameLine(label_spacing);
    ImGui::InputInt("##batch_size",
                    &hyperparams.batch_size,
                    1,
                    hyperparams.batch_size / 2);
    ImGui::SameLine();
    help_marker(R"(How many samples to record before updating the AI. 10 samples is one second.
Recommended: 4 - 1024)");

    ImGui::Text("Number of parallel\nenvironments:");
    ImGui::SameLine(label_spacing);
    ImGui::SliderInt("##num_env", &hyperparams.num_env, 1, 16);
    ImGui::SameLine();
    help_marker(R"(How many environments to run in parallel. More parallel environments stabilizes training and can use more CPU cores.
Recommended: 4 - 12)");

    ImGui::Text("Learning rate:");
    ImGui::SameLine(label_spacing);
    ImGui::InputFloat("##learning_rate",
                      &hyperparams.learning_rate,
                      hyperparams.learning_rate / 10,
                      hyperparams.learning_rate,
                      "%.5f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);
    hyperparams.learning_rate = std::clamp(hyperparams.learning_rate, 0.f, 1.f);
    ImGui::SameLine();
    help_marker(R"(How large of an update to make during each training step. Larger values will learn faster, but training will be more unstable.
Recommended: 0.00001 - 0.001)");

    ImGui::Text("Discount factor:");
    ImGui::SameLine(label_spacing);
    ImGui::InputFloat("##discount_factor",
                      &hyperparams.discount_factor,
                      hyperparams.discount_factor / 30,
                      hyperparams.discount_factor,
                      "%.5f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);
    hyperparams.discount_factor = std::clamp(hyperparams.discount_factor, 0.f, 1.f);
    ImGui::SameLine();
    help_marker(R"(How far ahead the body should consider when making decisions. 0 doesn't consider other than the immediate future. 1 will plan infinitely far ahead.
Recommended: 0.8 - 0.999)");

    ImGui::Text("Entropy bonus:");
    ImGui::SameLine(label_spacing);
    ImGui::InputFloat("##entropy_bonus",
                      &hyperparams.entropy_coef,
                      hyperparams.entropy_coef / 10,
                      hyperparams.entropy_coef,
                      "%.5f",
                      ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);
    ImGui::SameLine();
    help_marker(R"(Higher values encourage the body to explore its options rather than commiting to a strategy early in training.
Recommended: 0 - 0.1)");

    ImGui::Text("Actor-critic weight:");
    ImGui::SameLine(label_spacing);
    ImGui::SliderFloat("##actor_critic_weight",
                       &hyperparams.actor_loss_coef,
                       0,
                       1);
    hyperparams.value_loss_coef = 1. - hyperparams.actor_loss_coef;
    ImGui::SameLine();
    help_marker(R"(Weight training towards either the actor or the critic. High values make the actor more important, low values make the critic more important.
Recommended: 0.25 - 0.5)");

    if (hyperparams.algorithm == Algorithm::PPO)
    {
        ImGui::Text("PPO clipping factor:");
        ImGui::SameLine(label_spacing);
        ImGui::InputFloat("##ppo_clipping_factor",
                          &hyperparams.clip_param,
                          hyperparams.clip_param / 10,
                          hyperparams.clip_param,
                          "%.2f",
                          ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsScientific);
        ImGui::SameLine();
        help_marker(R"(A maximum limit on how much the probability of an action can change in one AI update.
Recommended: 0.1 - 0.2)");

        ImGui::Text("Epoch count:");
        ImGui::SameLine(label_spacing);
        ImGui::SliderInt("##epoch_count", &hyperparams.num_epoch, 1, 20);
        ImGui::SameLine();
        help_marker(R"(How many times to use each batch for an update. More epochs on a batch will be more sample efficient, but also less stable.
Recommended: 2 - 5)");

        ImGui::Text("Minibatch count:");
        ImGui::SameLine(label_spacing);
        ImGui::SliderInt("##minibatch_count", &hyperparams.num_minibatch, 1, 128);
        ImGui::SameLine();
        help_marker(R"(How many minibatches to split a batch into. More minibatches are more stable, but also slower.
Recommended: 8 - 32)");
    }

    ImGui::End();
}
}