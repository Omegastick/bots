#include <algorithm>
#include <filesystem>

#include <cpprl/cpprl.h>
#include <imgui.h>

#include "ui/training_wizard_screen/wizard_checkpoint_selector_window.h"
#include "misc/io.h"
#include <ui/training_wizard_screen/wizard_action.h>

namespace fs = std::filesystem;

namespace SingularityTrainer
{
WizardCheckpointSelectorWindow::WizardCheckpointSelectorWindow(IO &io)
    : last_selected_file(-1),
      selected_file(-1),
      io(&io) {}

WizardAction WizardCheckpointSelectorWindow::update(cpprl::Policy &policy)
{
    auto resolution = io->get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.333f, resolution.y * 0.5f});
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.05f});
    ImGui::Begin("Pick a checkpoint");

    // Enumerate all model files
    std::vector<std::string> files;
    for (const auto &file : fs::directory_iterator(fs::current_path()))
    {
        if (file.path().extension().string() == ".pth")
        {
            auto filename = file.path().filename().replace_extension("").string();
            files.push_back(filename);
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
    if (selected_file != last_selected_file)
    {
        auto nn_base = std::make_shared<cpprl::MlpBase>(23, false, 24);
        policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
        torch::load(policy, files[selected_file] + ".pth");
    }

    last_selected_file = selected_file;

    // Cancel, back, next
    auto action = WizardAction::None;

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() -
                         ImGui::GetTextLineHeight() * 2.5);
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x -
                         150 -
                         ImGui::GetStyle().ItemSpacing.x * 2);

    if (ImGui::Button("Cancel", {50, 20}))
    {
        action = WizardAction::Cancel;
    }

    ImGui::SameLine();

    if (ImGui::Button("Back", {50, 20}))
    {
        action = WizardAction::Back;
    }

    ImGui::SameLine();

    if (ImGui::Button("Next", {50, 20}))
    {
        action = WizardAction::Next;
    }

    ImGui::End();

    return action;
}
}