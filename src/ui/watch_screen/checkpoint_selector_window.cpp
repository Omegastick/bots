#include <algorithm>
#include <filesystem>

#include <cpprl/cpprl.h>
#include <imgui.h>

#include "ui/watch_screen/checkpoint_selector_window.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
CheckpointSelectorWindow::CheckpointSelectorWindow() : selected_file(-1) {}

std::unique_ptr<cpprl::Policy> CheckpointSelectorWindow::update()
{
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    ImGui::Begin("Pick a checkpoint", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

    // Enumerate all model files
    std::vector<std::string> files;
    for (const auto &file : fs::directory_iterator(fs::current_path()))
    {
        if (file.path().extension().string() == ".pth")
        {
            auto filename = file.path().filename().string();
            files.push_back(filename.substr(0, filename.size() - 4));
        }
    }
    std::sort(files.begin(), files.end());

    // Display list of model files
    std::vector<char *> c_strings;
    for (auto &string : files)
    {
        c_strings.push_back(&string.front());
    }
    ImGui::ListBox("", &selected_file, &c_strings.front(), files.size());

    // Load model
    std::unique_ptr<cpprl::Policy> policy = nullptr;
    if (ImGui::Button("Select"))
    {
        auto nn_base = std::make_shared<cpprl::MlpBase>(23, false, 24);
        policy = std::make_unique<cpprl::Policy>(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
        torch::load(*policy, files[selected_file] + ".pth");
    }
    ImGui::End();

    return policy;
}
}