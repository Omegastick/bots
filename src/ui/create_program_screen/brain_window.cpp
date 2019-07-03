#include <imgui.h>

#include "brain_window.h"
#include "misc/io.h"
#include "training/checkpointer.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
BrainWindow::BrainWindow(Checkpointer &checkpointer, IO &io)
    : checkpointer(checkpointer),
      io(io),
      last_selected_file(-1),
      selected_file(-1) {}

void BrainWindow::update(TrainingProgram &program)
{
    glm::vec2 resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.2f}, ImGuiCond_Once);
    ImGui::Begin("Brain");

    ImGui::Text("Checkpoint save frequency (minutes)");
    ImGui::InputInt("##minutes_per_checkpoint", &program.minutes_per_checkpoint, 1, 10);

    if (ImGui::IsWindowAppearing())
    {
        checkpoints.clear();
        auto all_paths = checkpointer.enumerate_checkpoints();
        for (const auto &path : all_paths)
        {
            if (checkpointer.load_data(path).body_spec["name"] == program.body["name"])
            {
                checkpoints.push_back(path);
            }
        }

        for (unsigned int i = 0; i < checkpoints.size(); ++i)
        {
            if (program.checkpoint == checkpoints[i])
            {
                selected_file = i + 1;
                last_selected_file = selected_file;
            }
        }
    }

    std::vector<const char *> c_strings{"None"};
    for (const auto &checkpoint : checkpoints)
    {
        c_strings.push_back(checkpoint.filename().replace_extension("").c_str());
    }
    ImGui::ListBox("", &selected_file, &c_strings.front(), c_strings.size());

    if (selected_file != last_selected_file)
    {
        if (selected_file == 0)
        {
            program.checkpoint = "";
        }
        else
        {
            program.checkpoint = checkpoints[selected_file - 1];
        }
        last_selected_file = selected_file;
    }

    ImGui::End();
}
}