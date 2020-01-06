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

using namespace ai;

namespace ImGui
{
constexpr double tick_factor_x = 0.4;
constexpr double tick_factor_y = 0.4;
constexpr double pixels_per_point = 5;

void Plot(const std::string &label,
          const std::vector<double> &raw_ys,
          const std::vector<double> &raw_xs,
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
        size.x = GetContentRegionAvail().x;
    }
    if (size.y <= 0)
    {
        size.y = GetContentRegionAvail().y;
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

    if (raw_xs.size() == 0)
    {
        return;
    }

    const auto x_minmax = std::minmax_element(raw_xs.begin(), raw_xs.end());
    const double x_min = *std::get<0>(x_minmax);
    const double x_max = *std::get<1>(x_minmax);
    const double x_range = x_max - x_min;
    const auto y_minmax = std::minmax_element(raw_ys.begin(), raw_ys.end());
    const double y_min = *std::get<0>(y_minmax);
    const double y_max = *std::get<1>(y_minmax);
    const double y_range = y_max - y_min;

    auto item_count = raw_ys.size();

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

    // Preprocess data
    std::vector<double> xs;
    std::vector<double> ys;
    if (static_cast<double>(item_count) <= size.x / pixels_per_point)
    {
        xs = raw_xs;
        ys = raw_ys;
    }
    else
    {
        const auto average_length = static_cast<double>(item_count) /
                                    (size.x / pixels_per_point);
        double accumulated_value = 0;
        double values_seen = 0;
        for (const auto i : indices(raw_xs))
        {
            values_seen++;
            accumulated_value += raw_ys[i];
            if (values_seen >= average_length)
            {
                xs.push_back(raw_xs[i]);
                const auto average = accumulated_value / values_seen;
                ys.push_back(average);
                values_seen = 0;
                accumulated_value = 0;
            }
        }
        item_count = xs.size();
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
        std::vector<ImVec2> points;
        for (auto i : indices(xs))
        {
            const auto x = ImLerp(inner_bb.Min.x,
                                  inner_bb.Max.x,
                                  static_cast<float>((xs[i] - x_min) / x_range));
            const auto y = ImLerp(inner_bb.Max.y,
                                  inner_bb.Min.y,
                                  static_cast<float>((ys[i] - y_min) / y_range));

            points.push_back({x, y});

            // Add circle if hovered
            if (hovered && hovered_idx == i)
            {
                window.DrawList->AddCircle({x, y},
                                           5.f,
                                           GetColorU32(ImGuiCol_PlotLines),
                                           12,
                                           2.f);
            }
        }
        window.DrawList->AddPolyline(points.data(),
                                     static_cast<int>(item_count),
                                     GetColorU32(ImGuiCol_PlotLines),
                                     false,
                                     1.5f);
    }
}
}