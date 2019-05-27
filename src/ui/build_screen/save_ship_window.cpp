#include <filesystem>
#include <fstream>
#include <string>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ui/build_screen/save_ship_window.h"
#include "misc/io.h"
#include "training/agents/agent.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
SaveShipWindow::SaveShipWindow(IO &io) : io(&io), name("") {}

bool SaveShipWindow::update(Agent &agent)
{
    bool saved = false;
    ImGui::Begin("Save ship");

    ImGui::InputText("Name", &name);

    if (ImGui::Button("Save"))
    {
        // Check ships directory exists
        auto ships_directory = fs::current_path();
        ships_directory += "/ships";
        if (!fs::exists(ships_directory))
        {
            spdlog::debug("Ships directory does not exist, making it now");
            fs::create_directories(ships_directory);
        }

        auto file_name = ships_directory;
        file_name += "/" + name + ".json";
        spdlog::debug("Saving ship to {}", file_name.c_str());

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