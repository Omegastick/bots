#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#include <algorithm>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>
#include <fmt/format.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>

#include "unlock_parts_window.h"
#include "graphics/backend/texture.h"
#include "graphics/colors.h"
#include "misc/credentials_manager.h"
#include "misc/ihttp_client.h"
#include "misc/io.h"
#include "misc/module_texture_store.h"
#include "misc/resource_manager.h"
#include "misc/utilities.h"

namespace SingularityTrainer
{
UnlockPartsWindow::UnlockPartsWindow(CredentialsManager &credentials_manager,
                                     IHttpClient &http_client,
                                     IO &io,
                                     ModuleTextureStore &module_texture_store,
                                     ResourceManager &resource_manager)
    : credentials_manager(credentials_manager),
      credits(0),
      http_client(http_client),
      io(io),
      module_texture_store(module_texture_store),
      resource_manager(resource_manager),
      selected_part(nullptr) {}

void UnlockPartsWindow::refresh_info(int timeout)
{
    std::thread([&, timeout] {
        auto owned_parts_promise = std::async([&, timeout] {
            auto response = http_client.post(st_cloud_base_url + "get_user",
                                             {{"username", credentials_manager.get_username()}});
            auto future_status = response.wait_for(std::chrono::seconds(timeout));
            if (future_status == std::future_status::timeout)
            {
                spdlog::error("Get user info request timed out after {} seconds", timeout);
            }

            auto json = response.get();
            if (!json.contains("modules") || !json.contains("credits"))
            {
                spdlog::error("Bad Json received: {}", json.dump());
                return std::vector<std::string>{};
            }

            {
                std::lock_guard lock_guard(credits_mutex);
                credits = json["credits"];
            }

            return json["modules"].get<std::vector<std::string>>();
        });

        auto all_parts_promise = std::async([&, timeout] {
            auto response = http_client.post(st_cloud_base_url + "get_all_modules");
            auto future_status = response.wait_for(std::chrono::seconds(timeout));
            if (future_status == std::future_status::timeout)
            {
                spdlog::error("Get all parts request timed out after {} seconds", timeout);
            }

            auto json = response.get();
            if (!json.contains("modules"))
            {
                spdlog::error("Bad Json received: {}", json.dump());
                return std::vector<std::tuple<std::string, long>>();
            }

            std::vector<std::tuple<std::string, long>> modules;
            for (const auto &json_module : json["modules"])
            {
                modules.push_back({json_module["name"], json_module["price"]});
            }

            return modules;
        });

        std::vector<std::string> owned_parts;
        std::vector<std::tuple<std::string, long>> all_parts;
        try
        {
            owned_parts = owned_parts_promise.get();
        }
        catch (const std::exception &exception)
        {
            spdlog::error("Error getting user info: {}", exception.what());
            return;
        }
        try
        {
            all_parts = all_parts_promise.get();
        }
        catch (const std::exception &exception)
        {
            spdlog::error("Error getting list of parts: {}", exception.what());
            return;
        }

        std::lock_guard lock_guard(parts_mutex);
        parts.clear();
        for (const auto &part : all_parts)
        {
            Part part_object{std::get<0>(part), false, std::get<1>(part)};
            if (std::any_of(owned_parts.begin(),
                            owned_parts.end(),
                            [&](const std::string &part_name) {
                                return part_name == std::get<0>(part);
                            }))
            {
                part_object.owned = true;
            }
            parts.push_back(part_object);
        }
        if (selected_part == nullptr)
        {
            selected_part = &parts[0];
        }
    })
        .detach();
}

bool UnlockPartsWindow::update(bool &show)
{
    if (!show)
    {
        return false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
    auto resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.5f, resolution.y * 0.4f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.5f, resolution.y * 0.5f},
                            ImGuiCond_Once,
                            {0.5f, 0.5f});
    ImGui::Begin("Unlock parts", &show);
    if (ImGui::IsWindowAppearing())
    {
        refresh_info();
    }
    ImGui::Columns(2);
    ImGui::BeginChild("1");

    ImGui::Text("Credits: %ld", credits);
    ImGui::Separator();

    const auto &style = ImGui::GetStyle();
    const float window_visible_x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    {
        std::lock_guard lock_guard(parts_mutex);
        const float image_size = resolution.x * 0.05f;
        for (unsigned int i = 0; i < parts.size(); ++i)
        {
            ImGui::BeginGroup();
            const auto &texture = module_texture_store.get(parts[i].name);
            const auto tint = parts[i].owned ? glm::vec4{1, 1, 1, 1}
                                             : glm::vec4{0.5f, 0.5f, 0.5f, 1};
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {13, 3});
            if (ImGui::ImageButton(ImTextureID(texture.get_id()),
                                   ImVec2(image_size, image_size),
                                   {1, 1},
                                   {0, 0},
                                   -1,
                                   {0, 0, 0, 0},
                                   tint))
            {
                selected_part = &parts[i];
            }
            ImGui::Text("%s", parts[i].name.c_str());
            {
                std::lock_guard lock_guard(credits_mutex);
                const auto color = credits >= parts[i].price ? cl_green : cl_red;
                ImGui::TextColored(color, "%ld", parts[i].price);
            }
            ImGui::PopStyleVar();
            ImGui::EndGroup();

            const float last_button_x = ImGui::GetItemRectMax().x;
            const float next_button_x = last_button_x + style.ItemSpacing.x + image_size;
            if (i + 1 < parts.size() && next_button_x < window_visible_x)
            {
                ImGui::SameLine();
            }
        }
    }
    ImGui::PopStyleVar();

    ImGui::EndChild();
    ImGui::NextColumn();

    if (selected_part != nullptr)
    {
        ImGui::BeginGroup();
        ImGui::Text("%s", selected_part->name.c_str());
        bool can_afford = credits >= selected_part->price ? true : false;
        {
            std::lock_guard lock_guard(credits_mutex);
            const auto color = can_afford ? cl_green : cl_red;
            ImGui::TextColored(color, "%ld", selected_part->price);
        }
        if (can_afford)
        {
            ImGui::Button("Buy");
        }
        else
        {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            ImGui::Button("Buy");
            ImGui::PopStyleVar();
        }
        ImGui::EndGroup();

        const float image_size = resolution.x * 0.1f;
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - image_size);
        const auto &texture = module_texture_store.get(selected_part->name);
        ImGui::Image(ImTextureID(texture.get_id()),
                     {image_size, image_size},
                     {1, 1},
                     {0, 0});
    }

    ImGui::End();

    return false;
}
}