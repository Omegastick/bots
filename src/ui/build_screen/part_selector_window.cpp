#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#include <string>
#include <vector>

#include <imgui.h>
#include <spdlog/spdlog.h>

#include "graphics/backend/texture.h"
#include "graphics/colors.h"
#include "resource_manager.h"
#include "ui/build_screen/part_selector_window.h"
#include "utilities.h"

namespace SingularityTrainer
{
PartSelectorWindow::PartSelectorWindow(ResourceManager &resource_manager)
    : resource_manager(&resource_manager)
{
}

std::string PartSelectorWindow::update(std::vector<std::string> &parts)
{
    std::string selected_part = "";
    ImGui::PushStyleColor(ImGuiCol_Button, glm_to_im(cl_base03));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, glm_to_im(cl_base03));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, glm_to_im(cl_base02));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
    ImGui::SetNextWindowSize({400, 600}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Part Selector");
    auto style = ImGui::GetStyle();
    auto image_size = ((ImGui::GetContentRegionAvailWidth() - style.ScrollbarSize * 2) / 3) - style.ItemSpacing.x;
    for (unsigned int i = 0; i < parts.size(); ++i)
    {
        if (i % 3 != 0)
        {
            ImGui::SameLine();
        }
        auto texture = resource_manager->texture_store.get(parts[i]);
        if (ImGui::ImageButton(ImTextureID(texture->get_id()), ImVec2(image_size, image_size), {1, 1}, {0, 0}))
        {
            selected_part = parts[i];
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    return selected_part;
}
}