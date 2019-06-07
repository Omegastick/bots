#include <imgui.h>

#include "algorithm_window.h"
#include "misc/io.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
AlgorithmWindow::AlgorithmWindow(IO &io)
    : io(io) {}

void AlgorithmWindow::update(HyperParameters &hyperparams)
{
    auto resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.5f}, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.2f}, ImGuiCond_Appearing);
    ImGui::Begin("Select an algorithm");

    const float label_spacing = resolution.x * 0.06;

    const char *algorithms[] = {"A2C", "PPO"};
    auto selected_algorithm = static_cast<int>(hyperparams.algorithm);
    ImGui::Text("Algorithm:");
    ImGui::SameLine(label_spacing);
    ImGui::Combo("##algorithm", &selected_algorithm, algorithms, 2);
    hyperparams.algorithm = static_cast<Algorithm>(selected_algorithm);

    ImGui::Text("Batch size:");
    ImGui::SameLine(label_spacing);
    float batch_size = hyperparams.batch_size;
    ImGui::SliderFloat("##batch_size", &batch_size, 1, 16384, "%.0f", 2);
    hyperparams.batch_size = std::round(batch_size);

    ImGui::End();
}
}