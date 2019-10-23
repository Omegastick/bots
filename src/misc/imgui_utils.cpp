#include <string>
#include <vector>

#include <imgui.h>

#include "imgui_utils.h"

namespace ImGui
{
bool ListBox(const char *label,
             int *current_item,
             std::vector<std::string> &items,
             int height_in_items)
{
    return ListBox(label,
                   current_item,
                   [](void *data, int idx, const char **out_text) {
                       *out_text = (*static_cast<const std::vector<std::string> *>(data))[idx].c_str();
                       return true;
                   },
                   static_cast<void *>(&items),
                   static_cast<int>(items.size()),
                   height_in_items);
}
}