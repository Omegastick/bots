#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "save_body_window.h"
#include "environment/build_env.h"
#include "misc/animator.h"
#include "misc/io.h"

namespace fs = std::filesystem;

namespace ai
{
SaveBodyWindow::SaveBodyWindow(Animator &animator, IO &io)
    : animation_id(std::numeric_limits<unsigned long>::max()),
      animator(animator),
      io(io),
      name(""),
      recently_saved(false) {}

SaveBodyWindow::~SaveBodyWindow()
{
    animator.delete_animation(animation_id);
}

bool SaveBodyWindow::update(BuildEnv &build_env)
{
    const auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowPos({resolution.x * 0.85f, resolution.y * 0.85f}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({resolution.x * 0.125f, resolution.y * 0.15f}, ImGuiCond_Once);
    ImGui::Begin("Save body");

    ImGui::InputText("##name", &name);

    bool saved = false;
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

        build_env.set_name(name);
        const auto json = build_env.serialize_body();
        std::ofstream file(file_name);
        file << json;
        saved = true;
        recently_saved = true;

        animator.delete_animation(animation_id);
        animation_id = animator.add_animation({[](double) {},
                                               1.,
                                               [&]() {
                                                   recently_saved = false;
                                               }});
    }

    if (recently_saved)
    {
        ImGui::Text("Saved");
    }

    ImGui::End();
    return saved;
}
}