#include <string>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <doctest/trompeloeil.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <spdlog/spdlog.h>

#include "screens/main_menu_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
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
                               IScreenFactory &build_screen_factory,
                               IScreenFactory &create_program_screen_factory,
                               IScreenFactory &multiplayer_screen_factory,
                               ScreenManager &screen_manager)
    : credentials_manager(credentials_manager),
      build_screen_factory(build_screen_factory),
      create_program_screen_factory(create_program_screen_factory),
      multiplayer_screen_factory(multiplayer_screen_factory),
      screen_manager(screen_manager) {}

void MainMenuScreen::update(double /*delta_time*/)
{
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    if (credentials_manager.get_token().empty())
    {
        // Show login window
        ImGui::Begin("Login", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputText("Username", &username);
        if (ImGui::Button("Login"))
        {
            credentials_manager.login(username);
            spdlog::debug("Logged in as: {}", username);
            spdlog::debug("Token: {}", credentials_manager.get_token());
        }
        ImGui::End();
    }
    else
    {
        // Show main menu
        ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});
        ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1, 1, 1, 0.1});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {1, 1, 1, 0.05});
        ImGui::PushStyleColor(ImGuiCol_Text, {cl_base3.r, cl_base3.g, cl_base3.b, 1});
        auto imgui_io = ImGui::GetIO();
        ImGui::PushFont(imgui_io.Fonts->Fonts[2]);
        ImGui::Begin("Main menu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
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
        ImGui::End();
        ImGui::PopFont();
        ImGui::PopStyleColor(5);
    }

    // ImGui::ShowStyleEditor();
    // ImGui::ShowDemoWindow();
}

void MainMenuScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();
    renderer.end();
}

void MainMenuScreen::build_body()
{
    screen_manager.show_screen(build_screen_factory.make());
}

void MainMenuScreen::multiplayer()
{
    screen_manager.show_screen(multiplayer_screen_factory.make());
}

void MainMenuScreen::train_agent()
{
    screen_manager.show_screen(create_program_screen_factory.make());
}

void MainMenuScreen::quit()
{
    screen_manager.close_screen();
}

TEST_CASE("MainMenuScreen")
{
    MockHttpClient http_client;
    CredentialsManager credentials_manager(http_client);
    MockScreenFactory build_screen_factory;
    MockScreenFactory create_program_screen_factory;
    MockScreenFactory multiplayer_screen_factory;
    ScreenManager screen_manager;
    auto main_menu_screen = std::make_shared<MainMenuScreen>(credentials_manager,
                                                             build_screen_factory,
                                                             create_program_screen_factory,
                                                             multiplayer_screen_factory,
                                                             screen_manager);
    screen_manager.show_screen(main_menu_screen);

    SUBCASE("build_body()")
    {
        auto build_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*build_screen, update(0));
        REQUIRE_CALL(build_screen_factory, make())
            .RETURN(build_screen);
        main_menu_screen->build_body();
        screen_manager.update(0);
        DOCTEST_CHECK(screen_manager.stack_size() == 2);
    }

    SUBCASE("multiplayer()")
    {
        auto multiplayer_screen = std::make_shared<MockScreen>();
        REQUIRE_CALL(*multiplayer_screen, update(0));
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