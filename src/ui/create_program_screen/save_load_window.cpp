#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "save_load_window.h"
#include "misc/io.h"
#include "training/training_program.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
SaveLoadWindow::SaveLoadWindow(IO &io)
    : io(io),
      name("") {}

bool SaveLoadWindow::update(TrainingProgram &program)
{
    int loaded = false;

    auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.2f}, ImGuiCond_Once);
    ImGui::Begin("Save/Load program");

    // Enumerate all files
    std::vector<std::string> files;
    auto programs_directory = fs::current_path();
    programs_directory += "/programs";
    if (!fs::exists(programs_directory))
    {
        fs::create_directories(programs_directory);
    }
    for (const auto &file : fs::directory_iterator(programs_directory))
    {
        if (file.path().extension().string() == ".json")
        {
            auto filename = file.path().filename().replace_extension("").string();
            files.push_back(filename);
        }
    }
    std::sort(files.begin(), files.end());

    // Display list of files
    std::vector<char *> c_strings;
    int selected_file = -1;
    int matched_file = -1;
    for (unsigned int i = 0; i < files.size(); ++i)
    {
        c_strings.push_back(&files[i].front());
        if (files[i] == name)
        {
            selected_file = i;
            matched_file = i;
        }
    }
    ImGui::ListBox("", &selected_file, &c_strings.front(), static_cast<int>(files.size()));
    if (selected_file != matched_file)
    {
        name = files[selected_file];
    }

    ImGui::InputText("##filename", &name);

    // Save
    if (ImGui::Button("Save"))
    {
        auto save_path = programs_directory;
        save_path += "/" + name + ".json";
        std::ofstream file(save_path);
        file << program.to_json().dump();
    }

    ImGui::SameLine();

    // Load
    if (ImGui::Button("Load"))
    {
        auto load_path = programs_directory;
        load_path += "/" + name + ".json";
        std::ifstream file(load_path);
        auto json = nlohmann::json::parse(file);
        program = TrainingProgram(json);
        loaded = true;
    }

    ImGui::End();

    return loaded;
}
}