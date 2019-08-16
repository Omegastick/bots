#pragma once

namespace ImGui
{
bool ListBox(const char *label,
             int *current_item,
             const std::vector<std::string> &items,
             int height_in_items = -1);
}