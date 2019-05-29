#include <filesystem>
#include <fstream>
#include <string>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ui/build_screen/save_body_window.h"
#include "misc/io.h"
#include "training/agents/agent.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
SaveBodyWindow::SaveBodyWindow(IO &io) : io(&io), name("") {}

bool SaveBodyWindow::update(Agent &agent)
{
    bool saved = false;
    ImGui::Begin("Save body");

    ImGui::InputText("Name", &name);

    if (ImGui::Button("Save"))
    {
        // Check bodys directory exists
        auto bodys_directory = fs::current_path();
        bodys_directory += "/bodys";
        if (!fs::exists(bodys_directory))
        {
            spdlog::debug("Bodys directory does not exist, making it now");
            fs::create_directories(bodys_directory);
        }

        auto file_name = bodys_directory;
        file_name += "/" + name + ".json";
        spdlog::debug("Saving body to {}", file_name.c_str());

        agent.set_name(name);
        auto json = agent.to_json().dump();
        std::ofstream file(file_name);
        file << json;
        saved = true;
    }

    ImGui::End();
    return saved;
}
}