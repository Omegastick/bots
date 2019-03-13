#include <string>
#include <sstream>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "spdlog/spdlog.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "graphics/window.h"
#include "graphics/renderers/renderer.h"
#include "graphics/screens/quad_screen.h"
#include "graphics/screens/texture_test_screen.h"
#include "graphics/screens/sprite_test_screen.h"
#include "graphics/screens/post_proc_screen.h"
#include "graphics/screens/particle_test_screen.h"
// #include "graphics/screens/scene_test_screen.h"
#include "graphics/screens/line_test_screen.h"
#include "graphics/screens/crt_test_screen.h"
#include "graphics/screens/text_test_screen.h"
#include "screen_manager.h"
#include "resource_manager.h"
#include "iscreen.h"

using namespace SingularityTrainer;

const int resolution_x = 1920;
const int resolution_y = 1080;
const std::string window_title = "Graphics Playground";
const int opengl_version_major = 4;
const int opengl_version_minor = 3;

void error_callback(int error, const char *description)
{
    spdlog::error("GLFW error: [{}] {}", error, description);
}

void reset_imgui_style()
{
    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();

    style.WindowRounding = 2.3;
    style.GrabRounding = style.FrameRounding = 2.3;
    style.ScrollbarRounding = 2.3;
    style.FrameBorderSize = 1.0;
    style.ItemSpacing.y = 6.5;

    style.Colors[ImGuiCol_Text] = {0.9f, 0.9f, 0.9f, 1.00f};
    style.Colors[ImGuiCol_TextDisabled] = {0.34509805f, 0.34509805f, 0.34509805f, 1.00f};
    style.Colors[ImGuiCol_WindowBg] = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
    style.Colors[ImGuiCol_ChildBg] = {0.23529413f, 0.24705884f, 0.25490198f, 0.00f};
    style.Colors[ImGuiCol_PopupBg] = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
    style.Colors[ImGuiCol_Border] = {0.33333334f, 0.33333334f, 0.33333334f, 0.50f};
    style.Colors[ImGuiCol_BorderShadow] = {0.15686275f, 0.15686275f, 0.15686275f, 0.00f};
    style.Colors[ImGuiCol_FrameBg] = {0.16862746f, 0.16862746f, 0.16862746f, 0.54f};
    style.Colors[ImGuiCol_FrameBgHovered] = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
    style.Colors[ImGuiCol_FrameBgActive] = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
    style.Colors[ImGuiCol_TitleBg] = {0.23529413f, 0.24705884f, 0.25490198f, 0.54f};
    style.Colors[ImGuiCol_TitleBgCollapsed] = {0.23529413f, 0.24705884f, 0.25490198f, 0.34f};
    style.Colors[ImGuiCol_TitleBgActive] = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
    style.Colors[ImGuiCol_MenuBarBg] = {0.27058825f, 0.28627452f, 0.2901961f, 0.80f};
    style.Colors[ImGuiCol_ScrollbarBg] = {0.27058825f, 0.28627452f, 0.2901961f, 0.60f};
    style.Colors[ImGuiCol_ScrollbarGrab] = {0.21960786f, 0.30980393f, 0.41960788f, 0.51f};
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
    style.Colors[ImGuiCol_ScrollbarGrabActive] = {0.13725491f, 0.19215688f, 0.2627451f, 0.91f};
    style.Colors[ImGuiCol_CheckMark] = {0.90f, 0.90f, 0.90f, 0.83f};
    style.Colors[ImGuiCol_SliderGrab] = {0.70f, 0.70f, 0.70f, 0.62f};
    style.Colors[ImGuiCol_SliderGrabActive] = {0.30f, 0.30f, 0.30f, 0.84f};
    style.Colors[ImGuiCol_Button] = {0.33333334f, 0.3529412f, 0.36078432f, 0.49f};
    style.Colors[ImGuiCol_ButtonHovered] = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
    style.Colors[ImGuiCol_ButtonActive] = {0.13725491f, 0.19215688f, 0.2627451f, 1.00f};
    style.Colors[ImGuiCol_Header] = {0.33333334f, 0.3529412f, 0.36078432f, 0.53f};
    style.Colors[ImGuiCol_HeaderHovered] = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
    style.Colors[ImGuiCol_HeaderActive] = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
    style.Colors[ImGuiCol_Separator] = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
    style.Colors[ImGuiCol_SeparatorHovered] = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
    style.Colors[ImGuiCol_SeparatorActive] = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
    style.Colors[ImGuiCol_ResizeGrip] = {1.00f, 1.00f, 1.00f, 0.05f};
    style.Colors[ImGuiCol_ResizeGripHovered] = {1.00f, 1.00f, 1.00f, 0.4f};
    style.Colors[ImGuiCol_ResizeGripActive] = {1.00f, 1.00f, 1.00f, 0.3f};
    style.Colors[ImGuiCol_PlotLines] = {0.61f, 0.61f, 0.61f, 1.00f};
    style.Colors[ImGuiCol_PlotLinesHovered] = {1.00f, 0.43f, 0.35f, 1.00f};
    style.Colors[ImGuiCol_PlotHistogram] = {0.90f, 0.70f, 0.00f, 1.00f};
    style.Colors[ImGuiCol_PlotHistogramHovered] = {1.00f, 0.60f, 0.00f, 1.00f};
    style.Colors[ImGuiCol_TextSelectedBg] = {0.18431373f, 0.39607847f, 0.79215693f, 0.90f};
}

void resize_window_callback(GLFWwindow *glfw_window, int x, int y)
{
    spdlog::debug("Resizing window to {}x{}", x, y);
    glViewport(0, 0, x, y);

    reset_imgui_style();
    ImGuiStyle &style = ImGui::GetStyle();
    float relative_scale = (float)x / (float)resolution_x;
    style.ScaleAllSizes(relative_scale);
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = relative_scale;

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
    window->get_renderer().resize(x, y);
}

void init_imgui(const int opengl_version_major, const int opengl_version_minor, GLFWwindow *window)
{
    spdlog::debug("Initializing ImGui");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    std::stringstream string_stream;
    string_stream << "#version " << opengl_version_major << opengl_version_minor << "0 core";
    std::string x = string_stream.str();
    ImGui_ImplOpenGL3_Init(string_stream.str().c_str());

    ImFontConfig font_config;
    font_config.OversampleH = 3;
    font_config.OversampleV = 3;
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->ClearFonts();
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 15, &font_config);

    reset_imgui_style();
}

int main(int argc, const char *argv[])
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");
    glfwSetErrorCallback(error_callback);

    // Create window
    Window window = Window(resolution_x, resolution_y, window_title, opengl_version_major, opengl_version_minor);

    ResourceManager resource_manager("assets/");

    // Screens
    spdlog::debug("Initializing screens");
    ScreenManager screen_manager;
    std::vector<std::shared_ptr<IScreen>> screens;
    std::vector<std::string> screen_names;
    spdlog::debug("Initializing quad test");
    screens.push_back(std::make_shared<QuadScreen>(&screen_manager, &screens, &screen_names));
    screen_names.push_back("Quad test");
    spdlog::debug("Initializing texture test");
    screens.push_back(std::make_shared<TextureTestScreen>(&screen_manager, &screens, &screen_names));
    screen_names.push_back("Texture test");
    spdlog::debug("Initializing sprite test");
    screens.push_back(std::make_shared<SpriteTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Sprite test");
    spdlog::debug("Initializing post processing test");
    screens.push_back(std::make_shared<PostProcScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Post processing test");
    spdlog::debug("Initializing particle test");
    screens.push_back(std::make_shared<ParticleTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Particle test");
    // spdlog::debug("Initializing scene test");
    // screens.push_back(std::make_shared<SceneTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    // screen_names.push_back("Scene test");
    spdlog::debug("Initializing line test");
    screens.push_back(std::make_shared<LineTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Line test");
    spdlog::debug("Initializing CRT test");
    screens.push_back(std::make_shared<CrtTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("CRT test");
    spdlog::debug("Initializing text test");
    screens.push_back(std::make_shared<TextTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Text test");
    screen_manager.show_screen(screens[0]);

    spdlog::debug("Initializing renderer");
    Renderer renderer(1920, 1080, resource_manager);
    window.set_renderer(&renderer);
    window.set_resize_callback(resize_window_callback);

    // ImGui
    init_imgui(opengl_version_major, opengl_version_minor, window.window);
    bool show_demo_window = true;

    float time = glfwGetTime();

    // Main loop
    while (!window.should_close())
    {
        // Time
        float new_time = glfwGetTime();
        float delta_time = new_time - time;
        time = new_time;

        // Input
        glfwPollEvents();

        // Update
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        screen_manager.update(delta_time);

        // Draw
        renderer.clear();

        screen_manager.draw(renderer, delta_time);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swap_buffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
