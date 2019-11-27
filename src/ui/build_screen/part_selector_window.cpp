#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#include <future>
#include <string>
#include <thread>
#include <vector>

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>
#include <fmt/format.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "graphics/backend/texture.h"
#include "graphics/colors.h"
#include "misc/credentials_manager.h"
#include "misc/ihttp_client.h"
#include "misc/io.h"
#include "misc/module_texture_store.h"
#include "misc/resource_manager.h"
#include "ui/build_screen/part_selector_window.h"
#include "misc/utilities.h"

namespace SingularityTrainer
{
PartSelectorWindow::PartSelectorWindow(CredentialsManager &credentials_manager,
                                       IHttpClient &http_client,
                                       IO &io,
                                       ModuleTextureStore &module_texture_store,
                                       ResourceManager &resource_manager)
    : credentials_manager(credentials_manager),
      http_client(http_client),
      io(io),
      module_texture_store(module_texture_store),
      resource_manager(resource_manager)
{
    refresh_parts();
}

void PartSelectorWindow::refresh_parts(int timeout)
{
    std::thread([&] {
        auto response = http_client.post(st_cloud_base_url + "get_user",
                                         {{"username", credentials_manager.get_username()}});
        auto future_status = response.wait_for(std::chrono::seconds(timeout));
        if (future_status == std::future_status::timeout)
        {
            throw std::runtime_error(
                fmt::format("Get user info request timed out after {} seconds", timeout));
        }

        auto json = response.get();
        if (!json.contains("modules"))
        {
            spdlog::error("Bad Json received: {}", json.dump());
            throw std::runtime_error("Bad Json received from 'get_user' request");
        }

        parts = json["modules"].get<std::vector<std::string>>();
    })
        .detach();
}

std::string PartSelectorWindow::update()
{
    std::string selected_part = "";
    ImGui::PushStyleColor(ImGuiCol_Button, cl_base03);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, cl_base03);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, cl_base02);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
    auto resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.95f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.775f, resolution.y * 0.025f},
                            ImGuiCond_Once);
    ImGui::Begin("Part Selector");
    const auto &style = ImGui::GetStyle();
    const float image_size = resolution.x * 0.05f;
    const float window_visible_x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    for (unsigned int i = 0; i < parts.size(); ++i)
    {
        const auto &texture = module_texture_store.get(parts[i]);
        if (ImGui::ImageButton(ImTextureID(texture.get_id()),
                               ImVec2(image_size, image_size),
                               {1, 1},
                               {0, 0}))
        {
            selected_part = parts[i];
        }

        const float last_button_x = ImGui::GetItemRectMax().x;
        const float next_button_x = last_button_x + style.ItemSpacing.x + image_size;
        if (i + 1 < parts.size() && next_button_x < window_visible_x)
        {
            ImGui::SameLine();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    return selected_part;
}
}