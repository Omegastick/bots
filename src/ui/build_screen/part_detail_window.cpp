#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "ui/build_screen/part_detail_window.h"
#include "io.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
PartDetailWindow::PartDetailWindow(IO &io) : selected_part(nullptr), io(&io) {}

void PartDetailWindow::select_part(IModule *part)
{
    selected_part = part;
}

void PartDetailWindow::update()
{
    auto resolution = io->get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.45f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.05f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Part Detail");

    if (selected_part != nullptr)
    {
        auto text = selected_part->to_json().dump(4);
        ImGui::Text("%s", text.c_str());
    }

    ImGui::End();
}
}