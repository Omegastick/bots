#include <algorithm>
#include <filesystem>
#include <fstream>

#include <Box2D/Box2D.h>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include "ui/training_wizard_screen/body_selector_window.h"
#include "misc/io.h"
#include "misc/random.h"
#include "training/agents/agent.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
BodySelectorWindow::BodySelectorWindow(IO &io)
    : last_selected_file(-1),
      selected_file(-1),
      io(&io) {}

bool BodySelectorWindow::update(Random &rng, b2World &b2_world, Agent &agent)
{
    // Load agent window
    auto resolution = io->get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.333f, resolution.y * 0.5f});
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.05f});
    ImGui::Begin("Pick a checkpoint");

    ImGui::Columns(2);

    // Enumerate all model files
    std::vector<std::string> files;
    auto bodies_directory = fs::current_path();
    bodies_directory += "/bodies";
    for (const auto &file : fs::directory_iterator(bodies_directory))
    {
        if (file.path().extension().string() == ".json")
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

    // Load body
    if (selected_file != last_selected_file)
    {
        auto json_file_path = bodies_directory;
        json_file_path += "/" + files[selected_file] + ".json";
        std::ifstream json_file(json_file_path);
        auto json = nlohmann::json::parse(json_file);
        agent = Agent(b2_world, &rng, json);
    }

    last_selected_file = selected_file;

    ImGui::NextColumn();

    if (agent.get_modules().size() > 0)
    {
        ImGui::Text("%s", agent.to_json().dump(2).c_str());
    }

    auto next = false;

    ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y -
                         ImGui::GetTextLineHeight() * 2);
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() -
                         ImGui::CalcTextSize("Next").x * 2);
    if (ImGui::Button("Next"))
    {
        next = true;
    }

    ImGui::End();

    return next;
}
}