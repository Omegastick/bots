#include <memory>
#include <filesystem>
#include <string>
#include <vector>

#include <cpprl/cpprl.h>
#include <imgui.h>

#include "choose_agent_window.h"
#include "graphics/imgui_utils.h"
#include "misc/io.h"
#include "training/agents/nn_agent.h"
#include "training/checkpointer.h"

namespace SingularityTrainer
{
ChooseAgentWindow::ChooseAgentWindow(Checkpointer &checkpointer, IO &io)
    : checkpointer(checkpointer),
      io(io),
      selected_file(-1) {}

std::unique_ptr<IAgent> ChooseAgentWindow::update()
{
    glm::vec2 resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    ImGui::Begin("Brain", NULL, ImGuiWindowFlags_NoResize);

    if (ImGui::IsWindowAppearing())
    {
        checkpoints.clear();
        auto all_paths = checkpointer.enumerate_checkpoints();
        for (const auto &path : all_paths)
        {
            checkpoints.push_back(path);
        }
    }

    std::vector<std::string> checkpoint_strings;
    std::transform(checkpoints.begin(),
                   checkpoints.end(),
                   std::back_inserter(checkpoint_strings),
                   [](const std::filesystem::path &path) {
                       return path.filename().replace_extension("").string();
                   });
    ImGui::ListBox("", &selected_file, checkpoint_strings);

    std::unique_ptr<IAgent> agent;
    bool done = false;
    if (selected_file == -1)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
        ImGui::Button("Select");
        ImGui::PopStyleVar();
    }
    else if (ImGui::Button("Select"))
    {
        std::filesystem::path path("./checkpoints/" + checkpoint_strings[selected_file] + ".meta");
        auto checkpoint = checkpointer.load(path);
        agent = std::make_unique<NNAgent>(checkpoint.policy, checkpoint.data.body_spec, "Player");
        done = true;
    }

    ImGui::End();

    if (done)
    {
        return agent;
    }
    return nullptr;
}
}