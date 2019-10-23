#include <vector>
#include <string>

#include <imgui.h>

#include "graphics/imgui_utils.h"

namespace ImGui
{
bool Combo(const char *label, int *currIndex, std::vector<std::string> &values)
{
    if (values.empty())
    {
        return false;
    }
    return Combo(label,
                 currIndex,
                 vector_getter,
                 static_cast<void *>(&values),
                 static_cast<int>(values.size()));
}

bool ListBox(const char *label, int *currIndex, std::vector<std::string> &values)
{
    if (values.empty())
    {
        return false;
    }
    return ListBox(label,
                   currIndex,
                   vector_getter,
                   static_cast<void *>(&values),
                   static_cast<int>(values.size()));
}
}