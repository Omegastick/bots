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
      resource_manager(resource_manager) {}

void PartSelectorWindow::refresh_parts(int timeout)
{
    std::thread([&, timeout] {
        auto response = http_client.post(st_cloud_base_url + "get_user",
                                         {{"username", credentials_manager.get_username()}});
        auto future_status = response.wait_for(std::chrono::seconds(timeout));
        if (future_status == std::future_status::timeout)
        {
            spdlog::error("Get user info request timed out after {} seconds", timeout);
        }

        nlohmann::json json;
        try
        {
            json = response.get();
        }
        catch (const std::exception &exception)
        {
            spdlog::error("Error getting list of owned parts: {}", exception.what());
            return;
        }
        if (!json.contains("modules"))
        {
            spdlog::error("Bad Json received: {}", json.dump());
            return;
        }

        parts = json["modules"].get<std::vector<std::string>>();
    })
        .detach();
}

std::string PartSelectorWindow::update(const std::string &selected_part,
                                       bool &show_unlock_parts_window)
{
    std::string new_selected_part = "";
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
    auto resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.775f, resolution.y * 0.025f},
                            ImGuiCond_Once);
    ImGui::Begin("Part Selector");
    if (ImGui::IsWindowAppearing())
    {
        refresh_parts();
    }
    const auto &style = ImGui::GetStyle();
    const float image_size = resolution.x * 0.05f;
    const float window_visible_x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    for (unsigned int i = 0; i < parts.size(); ++i)
    {
        const auto &texture = module_texture_store.get(parts[i]);
        bool active = false;
        if (parts[i] == selected_part)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, cl_base00);
            active = true;
        }
        if (ImGui::ImageButton(ImTextureID(texture.get_id()),
                               ImVec2(image_size, image_size),
                               {1, 1},
                               {0, 0}))
        {
            new_selected_part = parts[i];
        }
        if (active)
        {
            ImGui::PopStyleColor();
        }

        const float last_button_x = ImGui::GetItemRectMax().x;
        const float next_button_x = last_button_x + style.ItemSpacing.x + image_size;
        if (i + 1 < parts.size() && next_button_x < window_visible_x)
        {
            ImGui::SameLine();
        }
    }

    ImGui::PopStyleVar();

    if (ImGui::Button("Unlock parts"))
    {
        show_unlock_parts_window = true;
    }

    ImGui::End();

    return new_selected_part;
}
}