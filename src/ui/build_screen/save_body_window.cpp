#include <filesystem>
#include <fstream>
#include <string>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ui/build_screen/save_body_window.h"
#include "training/bodies/body.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
SaveBodyWindow::SaveBodyWindow() : name("") {}

bool SaveBodyWindow::update(Body &body)
{
    bool saved = false;
    ImGui::Begin("Save body");

    ImGui::InputText("Name", &name);

    if (ImGui::Button("Save"))
    {
        // Check bodies directory exists
        auto bodies_directory = fs::current_path();
        bodies_directory += "/bodies";
        if (!fs::exists(bodies_directory))
        {
            spdlog::debug("Bodies directory does not exist, making it now");
            fs::create_directories(bodies_directory);
        }

        auto file_name = bodies_directory;
        file_name += "/" + name + ".json";
        spdlog::debug("Saving body to {}", file_name.string());

        body.set_name(name);
        auto json = body.to_json().dump();
        std::ofstream file(file_name);
        file << json;
        saved = true;
    }

    ImGui::End();
    return saved;
}
}