#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>

#include <Box2D/Box2D.h>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include "ui/shared/body_selector_window.h"
#include "misc/random.h"
#include "training/agents/agent.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
BodySelectorWindow::BodySelectorWindow(IO &io) : selected_file(-1), io(&io) {}

std::unique_ptr<Agent> BodySelectorWindow::update(Random &rng, b2World &b2_world)
{
    // Load agent window
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    ImGui::Begin("Pick a checkpoint", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

    // Enumerate all model files
    std::vector<std::string> files;
    auto bodys_directory = fs::current_path();
    bodys_directory += "/bodys";
    for (const auto &file : fs::directory_iterator(bodys_directory))
    {
        if (file.path().extension().string() == ".json")
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
    std::unique_ptr<Agent> agent = nullptr;
    if (ImGui::Button("Select"))
    {
        auto json_file_path = bodys_directory;
        json_file_path += "/" + files[selected_file] + ".json";
        std::ifstream json_file(json_file_path);
        auto json = nlohmann::json::parse(json_file);
        agent = std::make_unique<Agent>(b2_world, &rng, json);
    }
    ImGui::End();

    return agent;
}
}