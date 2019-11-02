#include <cmath>
#include <vector>

#include <fmt/format.h>
#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

#include "plot.h"
#include "misc/utils/range.h"

using namespace SingularityTrainer;

namespace ImGui
{
constexpr double tick_factor_x = 0.4;
constexpr double tick_factor_y = 0.4;

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

    if (xs.size() == 0)
    {
        return;
    }

    const auto x_minmax = std::minmax_element(xs.begin(), xs.end());
    const double x_min = *std::get<0>(x_minmax);
    const double x_max = *std::get<1>(x_minmax);
    const double x_range = x_max - x_min;
    const auto y_minmax = std::minmax_element(ys.begin(), ys.end());
    const double y_min = *std::get<0>(y_minmax);
    const double y_max = *std::get<1>(y_minmax);
    const double y_range = y_max - y_min;

    const auto item_count = ys.size();

    const auto tick_width = std::pow(10, std::floor(std::log10(x_range))) * tick_factor_x;
    const auto tick_height = std::pow(10, std::floor(std::log10(y_range))) * tick_factor_y;

    // Draw X ticks
    {
        const float y0 = inner_bb.Min.y;
        const float y1 = inner_bb.Max.y;
        int counter = 0;
        for (double x_value = x_min; x_value < x_max; x_value += tick_width)
        {
            const float x = ImLerp(inner_bb.Min.x,
                                   inner_bb.Max.x,
                                   static_cast<float>((x_value - x_min) / x_range));

            const float alpha = counter++ % 5 == 0 ? 0.8f : 0.2f;
            window.DrawList->AddLine({x, y0},
                                     {x, y1},
                                     GetColorU32(ImGuiCol_TextDisabled, alpha));
        }
        window.DrawList->AddLine({inner_bb.Max.x, y0},
                                 {inner_bb.Max.x, y1},
                                 GetColorU32(ImGuiCol_TextDisabled));
    }

    // Draw Y ticks
    {
        const float x0 = inner_bb.Min.x;
        const float x1 = inner_bb.Max.x;
        int counter = 0;
        for (double y_value = y_min; y_value < y_max; y_value += tick_height)
        {
            const float y = ImLerp(inner_bb.Min.y,
                                   inner_bb.Max.y,
                                   static_cast<float>((y_value - y_min) / y_range));
            const float alpha = counter++ % 5 == 0 ? 0.8f : 0.2f;
            window.DrawList->AddLine({x0, y},
                                     {x1, y},
                                     GetColorU32(ImGuiCol_TextDisabled, alpha));
        }
        window.DrawList->AddLine({x0, inner_bb.Max.y},
                                 {x1, inner_bb.Max.y},
                                 GetColorU32(ImGuiCol_TextDisabled));
    }

    // Hover tooltip
    const auto id = window.GetID(label.c_str());
    const auto hovered = ItemHoverable(frame_bb, id);
    std::size_t hovered_idx{0};
    if (hovered)
    {
        const auto cursor = g.IO.MousePos;
        const auto scaled_cursor_x = ImClamp(
            (cursor.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x),
            0.0f,
            0.9999f);
        for (auto i : indices(xs))
        {
            hovered_idx = i;
            const auto scaled_x = (xs[i] - x_min) / x_range;
            if (scaled_x > scaled_cursor_x)
            {
                break;
            }
        }
        const std::string text = fmt::format("{:g}: {:g}", xs[hovered_idx], ys[hovered_idx]);
        SetTooltip("%s", text.c_str());
    }

    // Plot line
    {
        for (auto i : range(0ul, item_count - 1))
        {
            const auto x = xs[i];
            const auto y = ys[i];
            const auto x_next = xs[i + 1];
            const auto y_next = ys[i + 1];

            const auto x0 = ImLerp(inner_bb.Min.x,
                                   inner_bb.Max.x,
                                   static_cast<float>((x - x_min) / x_range));
            const auto x1 = ImLerp(inner_bb.Min.x,
                                   inner_bb.Max.x,
                                   static_cast<float>((x_next - x_min) / x_range));
            const auto y0 = ImLerp(inner_bb.Max.y,
                                   inner_bb.Min.y,
                                   static_cast<float>((y - y_min) / y_range));
            const auto y1 = ImLerp(inner_bb.Max.y,
                                   inner_bb.Min.y,
                                   static_cast<float>((y_next - y_min) / y_range));

            window.DrawList->AddLine({x0, y0}, {x1, y1}, GetColorU32(ImGuiCol_PlotLines), 3.f);

            // Add circle if hovered
            if (hovered && hovered_idx == i)
            {
                window.DrawList->AddCircle({x0, y0},
                                           5.f,
                                           GetColorU32(ImGuiCol_PlotLines),
                                           12,
                                           2.f);
            }
        }
    }
}
}