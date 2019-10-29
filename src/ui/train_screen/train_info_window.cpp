#include <map>
#include <vector>
#include <unordered_map>

#include <imgui.h>

#include "train_info_window.h"
#include "misc/io.h"

namespace SingularityTrainer
{
TrainInfoWindow::TrainInfoWindow(IO &io) : io(io) {}

void TrainInfoWindow::add_graph_data(const std::string &label,
                                     unsigned long long timestep,
                                     float value)
{
    data[label][timestep] = value;
}

void TrainInfoWindow::update(unsigned long long timestep, unsigned int update)
{
    auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.35f}, ImGuiSetCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.7f, resolution.y * 0.05f}, ImGuiSetCond_Once);
    ImGui::Begin("Training information");
    ImGui::Text("Update %i - Frame %lli", update, timestep);
    for (const auto &[category_name, category_data] : data)
    {
        for (const auto &[datum_timestep, datum] : category_data)
        {
            ImGui::Text("%s - %lli: %f", category_name.c_str(), datum_timestep, datum);
        }
    }
    ImGui::End();
}
}