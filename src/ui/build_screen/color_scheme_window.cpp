#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "color_scheme_window.h"
#include "graphics/colors.h"
#include "misc/io.h"
#include "training/bodies/body.h"

namespace ai
{
ColorSchemeWindow::ColorSchemeWindow(IO &io) : io(&io), selected_swatch(0) {}

bool ColorSchemeWindow::update(ColorScheme &color_scheme)
{
    auto resolution = static_cast<glm::vec2>(io->get_resolution());
    ImGui::SetNextWindowSize({0, resolution.y * 0.225f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.025f, resolution.y * 0.575f}, ImGuiCond_Once);
    ImGui::Begin("Color Scheme", nullptr, ImGuiWindowFlags_NoResize);

    if (ImGui::RadioButton("Primary", selected_swatch == 0))
    {
        selected_swatch = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Secondary", selected_swatch == 1))
    {
        selected_swatch = 1;
    }

    auto &color = selected_swatch == 0 ? color_scheme.primary : color_scheme.secondary;
    auto old_color = color;
    ImGui::ColorPicker4("##color_scheme_picker", glm::value_ptr(color),
                        ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview |
                            ImGuiColorEditFlags_RGB);

    ImGui::End();

    return color != old_color;
}
}