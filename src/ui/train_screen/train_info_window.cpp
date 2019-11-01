#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <vector>
#include <unordered_map>

#include <doctest.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "train_info_window.h"
#include "misc/io.h"
#include "misc/utils/range.h"

constexpr unsigned long long max_double_integer = std::pow(2, 53);

namespace SingularityTrainer
{
TrainInfoWindow::TrainInfoWindow(IO &io) : io(io) {}

void TrainInfoWindow::add_data(const std::string &label,
                               unsigned long long timestep,
                               double value)
{
    data[label].add_data(timestep, value);
}

void TrainInfoWindow::update(unsigned long long timestep, unsigned int update)
{
    auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowSize({resolution.x * 0.25f, resolution.y * 0.35f}, ImGuiSetCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.7f, resolution.y * 0.05f}, ImGuiSetCond_Once);
    ImGui::Begin("Training information");
    ImGui::Text("Update %i - Frame %lli", update, timestep);

    const auto &entropies = data["Entropy"].get_data().second;
    std::vector<float> entropy_floats(entropies.begin(), entropies.end());
    ImGui::PlotLines("Entropy",
                     entropy_floats.data(),
                     entropy_floats.size(),
                     0,
                     nullptr,
                     FLT_MAX,
                     FLT_MAX,
                     {300, 300});

    ImGui::End();
}

void IndexedDataStore::add_data(unsigned long long timestep, double value)
{
    if (timestep > max_double_integer)
    {
        spdlog::warn("Timestep too large to fit in a double: {}", timestep);
    }
    add_data(static_cast<double>(timestep), value);
}

void IndexedDataStore::add_data(double timestep, double value)
{
    if (indices.empty() || timestep > indices.back())
    {
        indices.push_back(timestep);
        values.push_back(value);
    }
    else
    {
        unsigned long index = 0;
        for (auto i : SingularityTrainer::indices(indices))
        {
            if (indices[i] > timestep)
            {
                index = i;
                break;
            }
        }
        indices.insert(indices.begin() + index, timestep);
        values.insert(values.begin() + index, value);
    }
}

const IndexedData IndexedDataStore::get_data() const
{
    return {indices, values};
}

TEST_CASE("IndexedDataStore")
{
    IndexedDataStore data_store;

    SUBCASE("Starts empty")
    {
        const auto &[indices, values] = data_store.get_data();
        DOCTEST_CHECK(indices.size() == 0);
        DOCTEST_CHECK(values.size() == 0);
    }

    SUBCASE("add_data()")
    {
        SUBCASE("Adds data")
        {
            data_store.add_data(5ull, 10.f);

            const auto &[indices, values] = data_store.get_data();
            DOCTEST_CHECK(indices.size() == 1);
            DOCTEST_CHECK(values.size() == 1);
            DOCTEST_CHECK(indices[0] == 5);
            DOCTEST_CHECK(values[0] == 10.f);
        }

        SUBCASE("Adds data to end of list")
        {
            data_store.add_data(5ull, 10.f);
            data_store.add_data(7ull, 8.f);
            data_store.add_data(9ull, 1000.f);

            const auto &[indices, values] = data_store.get_data();
            DOCTEST_CHECK(indices.size() == 3);
            DOCTEST_CHECK(values.size() == 3);
            DOCTEST_CHECK(indices[0] == 5);
            DOCTEST_CHECK(values[0] == 10.f);
            DOCTEST_CHECK(indices[1] == 7);
            DOCTEST_CHECK(values[1] == 8.f);
            DOCTEST_CHECK(indices[2] == 9);
            DOCTEST_CHECK(values[2] == 1000.f);
        }

        SUBCASE("Inserts data at correct timestep")
        {
            data_store.add_data(5ull, 10.f);
            data_store.add_data(7ull, 8.f);
            data_store.add_data(9ull, 1000.f);
            data_store.add_data(8ull, -6.f);

            const auto &[indices, values] = data_store.get_data();
            DOCTEST_CHECK(indices.size() == 4);
            DOCTEST_CHECK(values.size() == 4);
            DOCTEST_CHECK(indices[2] == 8);
            DOCTEST_CHECK(values[2] == -6.f);
        }
    }
}
}