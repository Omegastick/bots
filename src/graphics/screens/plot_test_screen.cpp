#include <cmath>
#include <vector>
#include <memory>
#include <string>

#include "plot_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "misc/utils/range.h"
#include "screens/iscreen.h"
#include "ui/plot.h"

namespace SingularityTrainer
{

PlotTestScreen::PlotTestScreen(
    ScreenManager &screen_manager,
    std::vector<std::shared_ptr<IScreen>> &screens,
    std::vector<std::string> &screen_names)
    : screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager)
{
    for (const auto i : range(0, 100))
    {
        xs.push_back(i * i);
        ys.push_back(std::sin(static_cast<double>(i) * 0.1f));
    }
}

void PlotTestScreen::update(double delta_time)
{
    display_test_dialog("Plot test", screens, screen_names, delta_time, screen_manager);

    const auto &io = ImGui::GetIO();
    ImGui::SetNextWindowPos({io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f}, 
                            ImGuiCond_Once,
                            {0.5, 0.5f});
    ImGui::SetNextWindowSize({960, 540}, ImGuiCond_Once);
    ImGui::Begin("Plot test");
    ImGui::Plot("Plot", ys, xs, {0, 400});
    ImGui::End();
}

void PlotTestScreen::draw(Renderer & /*renderer*/, bool /*lightweight*/) {}
}