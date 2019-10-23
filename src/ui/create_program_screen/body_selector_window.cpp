#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "body_selector_window.h"
#include "misc/io.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
BodySelectorWindow::BodySelectorWindow(IO &io)
    : io(io),
      last_selected_file(-1),
      selected_file(-1) {}

nlohmann::json BodySelectorWindow::update()
{
    auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.2f}, ImGuiCond_Once);
    ImGui::Begin("Pick a body");

    // Enumerate all model files
    std::vector<std::string> files;
    auto bodies_directory = fs::current_path();
    bodies_directory += "/bodies";
    if (!fs::exists(bodies_directory))
    {
        fs::create_directories(bodies_directory);
    }
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
    ImGui::ListBox("", &selected_file, &c_strings.front(), static_cast<int>(files.size()));

    // If the user selected a file, return the Json for that file
    nlohmann::json json;
    if (selected_file != last_selected_file)
    {
        auto json_file_path = bodies_directory;
        json_file_path += "/" + files[selected_file] + ".json";
        std::ifstream json_file(json_file_path);
        json = nlohmann::json::parse(json_file);
    }

    last_selected_file = selected_file;

    ImGui::End();

    return json;
}
}