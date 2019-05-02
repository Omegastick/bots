#include <imgui.h>

#include "ui/build_screen/part_selector_window.h"

namespace SingularityTrainer
{
PartSelectorWindow::PartSelectorWindow() {}

void PartSelectorWindow::update() {
    ImGui::Begin("Part Selector");
    ImGui::Text("I am the part selector");
    ImGui::End();
}
}