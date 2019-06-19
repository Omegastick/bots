#include <imgui.h>

#include "brain_window.h"
#include "misc/io.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
BrainWindow::BrainWindow(IO &io) : io(io) {}

void BrainWindow::update(TrainingProgram &program)
{
    glm::vec2 resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.25, resolution.y * 0.1f});
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.2f}, ImGuiCond_Appearing);
    ImGui::Begin("Brain");

    ImGui::Text("Checkpoint save frequency (minutes)");
    ImGui::InputInt("##minutes_per_checkpoint", &program.minutes_per_checkpoint, 1, 10);

    ImGui::End();
}
}