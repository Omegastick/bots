#include <cmath>
#include <vector>

#include <fmt/format.h>
#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

#include "plot.h"

namespace ImGui
{
constexpr double tick_factor_x = 0.3;
constexpr double tick_factor_y = 0.3;

void Plot(const std::string &label,
          const std::vector<double> &ys,
          const std::vector<double> &xs,
          ImVec2 size)
{
    ImGuiWindow &window = *GetCurrentWindow();
    if (window.SkipItems)
    {
        return;
    }

    // Calculate size
    if (size.x <= 0)
    {
        size.x = GetContentRegionAvailWidth();
    }

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window.GetID(label.c_str());

    const ImRect frame_bb(
        window.DC.CursorPos,
        window.DC.CursorPos + size);
    const ImRect inner_bb(
        frame_bb.Min + style.FramePadding,
        frame_bb.Max - style.FramePadding);
    const ImRect total_bb = frame_bb;
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0, &frame_bb))
    {
        return;
    }

    RenderFrame(
        frame_bb.Min,
        frame_bb.Max,
        GetColorU32(ImGuiCol_FrameBg),
        true,
        style.FrameRounding);

    const auto x_minmax = std::minmax_element(xs.begin(), xs.end());
    const double x_min = *std::get<0>(x_minmax);
    const double x_max = *std::get<1>(x_minmax);
    const double x_range = x_max - x_min;
    const auto y_minmax = std::minmax_element(ys.begin(), ys.end());
    const double y_min = *std::get<0>(y_minmax);
    const double y_max = *std::get<1>(y_minmax);
    const double y_range = y_max - y_min;

    const auto tick_width = std::pow(10, std::floor(std::log10(x_range))) * tick_factor_x;
    const auto tick_height = std::pow(10, std::floor(std::log10(y_range))) * tick_factor_y;

    // Draw X ticks
    {
        const float y0 = inner_bb.Min.y;
        const float y1 = inner_bb.Max.y;
        for (double x_value = x_min; x_value < x_max; x_value += tick_width)
        {
            const float x = ImLerp(inner_bb.Min.x,
                                   inner_bb.Max.x,
                                   static_cast<float>(x_value / x_max));
            window.DrawList->AddLine({x, y0},
                                     {x, y1},
                                     GetColorU32(ImGuiCol_TextDisabled));
        }
        window.DrawList->AddLine({inner_bb.Min.x, y0},
                                 {inner_bb.Max.x, y1},
                                 GetColorU32(ImGuiCol_TextDisabled));
    }

    // Draw Y ticks
    {
        const float x0 = inner_bb.Min.x;
        const float x1 = inner_bb.Max.x;
        for (double y_value = y_min; y_value < y_max; y_value += tick_height)
        {
            const float y = ImLerp(inner_bb.Min.y,
                                   inner_bb.Max.y,
                                   static_cast<float>(y_value / y_max));
            window.DrawList->AddLine({x0, y},
                                     {x1, y},
                                     GetColorU32(ImGuiCol_TextDisabled));
        }
        window.DrawList->AddLine({x0, inner_bb.Min.y},
                                 {x1, inner_bb.Max.y},
                                 GetColorU32(ImGuiCol_TextDisabled));
    }
}
}