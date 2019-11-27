#include <chrono>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <doctest.h>
#include <doctest/trompeloeil.hpp>
#include <fmt/format.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <spdlog/spdlog.h>

#include "screens/main_menu_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "misc/credentials_manager.h"
#include "misc/ihttp_client.h"
#include "misc/io.h"
#include "misc/resource_manager.h"
#include "screens/iscreen.h"
#include "training/training_program.h"
#include "misc/screen_manager.h"

namespace SingularityTrainer
{
MainMenuScreen::MainMenuScreen(CredentialsManager &credentials_manager,
                               IHttpClient &http_client,
                               IO &io,
                               IScreenFactory &build_screen_factory,
                               IScreenFactory &create_program_screen_factory,
                               IScreenFactory &multiplayer_screen_factory,
                               ScreenManager &screen_manager)
    : credentials_manager(credentials_manager),
      http_client(http_client),
      io(io),
      build_screen_factory(build_screen_factory),
      create_program_screen_factory(create_program_screen_factory),
      multiplayer_screen_factory(multiplayer_screen_factory),
      screen_manager(screen_manager),
      user_info_received(false),
      waiting_for_server(false) {}

void MainMenuScreen::update(double /*delta_time*/)
{
    const auto &imgui_io = ImGui::GetIO();
    ImGui::SetNextWindowPos({imgui_io.DisplaySize.x * 0.5f, imgui_io.DisplaySize.y * 0.5f},
                            ImGuiCond_Always,
                            {0.5, 0.5f});
    if (credentials_manager.get_token().empty())
    {
        // Show login window
        ImGui::Begin("Login", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputText("Username", &username);
        if (ImGui::Button("Login"))
        {
            std::thread([&] {
                credentials_manager.login(username);
                spdlog::debug("Logged in as: {}", username);
                user_info_future = get_user_info(st_cloud_base_url);
            })
                .detach();
            waiting_for_server = true;
        }
        if (waiting_for_server)
        {
            ImGui::SameLine();
            ImGui::Text("Please wait...");
        }
        ImGui::End();

        if (io.get_key_pressed(GLFW_KEY_LEFT_CONTROL) && io.get_key_pressed(GLFW_KEY_SPACE))
        {
            credentials_manager.set_token("No token");
        }
    }
    else
    {
        // Show main menu
        ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});
        ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1, 1, 1, 0.1f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {1, 1, 1, 0.05f});
        ImGui::PushStyleColor(ImGuiCol_Text, {cl_base3.r, cl_base3.g, cl_base3.b, 1});
        auto imgui_io = ImGui::GetIO();
        ImGui::PushFont(imgui_io.Fonts->Fonts[2]);
        ImGui::Begin("Main menu",
                     NULL,
                     (ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_AlwaysAutoResize |
                      ImGuiWindowFlags_NoTitleBar));
        if (ImGui::Button("Train Agent"))
        {
            train_agent();
        }
        if (ImGui::Button("Build Body"))
        {
            build_body();
        }
        if (ImGui::Button("Multiplayer"))
        {
            multiplayer();
        }
        if (ImGui::Button("Quit"))
        {
            quit();
        }

        if (user_info_received)
        {
            ImGui::Text("Credits: %ld", user_info.credits);
            ImGui::Text("Elo: %ld", user_info.elo);
        }
        else if (user_info_future.valid())
        {
            auto future_status = user_info_future.wait_for(std::chrono::seconds(0));
            if (future_status == std::future_status::ready)
            {
                user_info = user_info_future.get();
                user_info_received = true;
            }
        }
        ImGui::End();
        ImGui::PopFont();
        ImGui::PopStyleColor(5);
    }

    // ImGui::ShowStyleEditor();
    // ImGui::ShowDemoWindow();
}

void MainMenuScreen::draw(Renderer & /*renderer*/, bool /*lightweight*/)
{
}

void MainMenuScreen::build_body()
{
    screen_manager.show_screen(build_screen_factory.make());
}

std::future<MainMenuScreen::UserInfo> MainMenuScreen::get_user_info(const std::string &base_url,
                                                                    int timeout)
{
    return std::async(
        std::launch::async,
        [=] {
            auto response = http_client.post(base_url + "get_user",
                                             {{"username", credentials_manager.get_username()}});
            auto future_status = response.wait_for(std::chrono::seconds(timeout));
            if (future_status == std::future_status::timeout)
            {
                throw std::runtime_error(
                    fmt::format("Get user info request timed out after {} seconds", timeout));
            }

            auto json = response.get();
            if (!json.contains("elo") || !json.contains("credits"))
            {
                spdlog::error("Bad Json received: {}", json.dump());
                throw std::runtime_error("Bad Json received from 'get_user' request");
            }

            double elo_decimal = json["elo"];
            spdlog::debug("Elo received: {}", elo_decimal);

            long credits = static_cast<long>(json["credits"]);
            spdlog::debug("Credits received: {}", credits);

            return MainMenuScreen::UserInfo{credits, static_cast<long>(std::round(elo_decimal))};
        });
}

void MainMenuScreen::multiplayer()
{
    screen_manager.show_screen(multiplayer_screen_factory.make());
}

void MainMenuScreen::on_show()
{
    if (!credentials_manager.get_username().empty())
    {
        user_info_received = false;
        user_info_future = get_user_info(st_cloud_base_url);
    }
}

void MainMenuScreen::train_agent()
{
    screen_manager.show_screen(create_program_screen_factory.make());
}

void MainMenuScreen::quit()
{
    screen_manager.close_screen();
}

using trompeloeil::_;

TEST_CASE("MainMenuScreen")
{
    MockHttpClient http_client;
    CredentialsManager credentials_manager(http_client);
    IO io;
    MockScreenFactory build_screen_factory;
    MockScreenFactory create_program_screen_factory;
    MockScreenFactory multiplayer_screen_factory;
    ScreenManager screen_manager;
    auto main_menu_screen = std::make_shared<MainMenuScreen>(credentials_manager,
                                                             http_client,
                                                             io,
                                                             build_screen_factory,
                                                             create_program_screen_factory,
                                                             multiplayer_screen_factory,
                                                             screen_manager);
    screen_manager.show_screen(main_menu_screen);

    SUBCASE("build_body()")
    {
        auto build_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*build_screen, on_show());
        REQUIRE_CALL(*build_screen, update(0));
        REQUIRE_CALL(build_screen_factory, make())
            .RETURN(build_screen);
        main_menu_screen->build_body();
        screen_manager.update(0);
        DOCTEST_CHECK(screen_manager.stack_size() == 2);
    }

    SUBCASE("get_user_info()")
    {
        SUBCASE("Returns correct info")
        {
            credentials_manager.set_username("test_username");
            std::promise<nlohmann::json> promise;
            REQUIRE_CALL(http_client, post("http://asd.com/get_user",
                                           nlohmann::json{{"username", "test_username"}},
                                           _))
                .LR_RETURN(promise.get_future());
            promise.set_value(nlohmann::json{{"credits", 321}, {"elo", 123}});
            auto user_info_future = main_menu_screen->get_user_info("http://asd.com/");

            const auto user_info = user_info_future.get();
            CHECK(user_info.credits == 321);
            CHECK(user_info.elo == 123);
        }

        SUBCASE("Sets exception on timeout")
        {
            credentials_manager.set_username("test_username");
            std::promise<nlohmann::json> promise;
            REQUIRE_CALL(http_client, post("http://asd.com/get_user",
                                           nlohmann::json{{"username", "test_username"}},
                                           _))
                .LR_RETURN(promise.get_future());
            auto user_info_future = main_menu_screen->get_user_info("http://asd.com/", 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            promise.set_value(nlohmann::json{{"credits", 321}, {"elo", 123}});

            CHECK_THROWS(user_info_future.get());
        }
    }

    SUBCASE("multiplayer()")
    {
        auto multiplayer_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*multiplayer_screen, update(0));
        REQUIRE_CALL(*multiplayer_screen, on_show());
        REQUIRE_CALL(multiplayer_screen_factory, make())
            .RETURN(multiplayer_screen);
        main_menu_screen->multiplayer();
        screen_manager.update(0);
        DOCTEST_CHECK(screen_manager.stack_size() == 2);
    }

    SUBCASE("train_agent()")
    {
        auto create_program_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*create_program_screen, update(0));
        REQUIRE_CALL(*create_program_screen, on_show());
        REQUIRE_CALL(create_program_screen_factory, make())
            .RETURN(create_program_screen);
        main_menu_screen->train_agent();
        screen_manager.update(0);
        DOCTEST_CHECK(screen_manager.stack_size() == 2);
    }

    SUBCASE("quit()")
    {
        main_menu_screen->quit();
        screen_manager.update(0);
        DOCTEST_CHECK(screen_manager.stack_size() == 0);
    }
}
}