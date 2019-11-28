#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{
bool Spinner(const char *label, float radius, float thickness, const ImVec4 &color)
{
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size((radius)*2, (radius + style.FramePadding.y) * 2);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    // Render
    window->DrawList->PathClear();

    const float num_segments = 30;
    const float start = abs(ImSin(static_cast<float>(g.Time) * 1.8f) * (num_segments - 5));

    const float a_min = IM_PI * 2.0f * start / num_segments;
    const float a_max = IM_PI * 2.0f * (num_segments - 3) / num_segments;

    const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

    for (float i = 0; i < num_segments; i++)
    {
        const float a = a_min + (i / num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(
            ImVec2(centre.x + ImCos(a + static_cast<float>(g.Time) * 8) * radius,
                   centre.y + ImSin(a + static_cast<float>(g.Time) * 8) * radius));
    }

    window->DrawList->PathStroke(ImGui::ColorConvertFloat4ToU32(color), false, thickness);

    return true;
}
}