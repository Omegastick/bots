#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <argh.h>
#include <curlpp/cURLpp.hpp>
#include <doctest.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>

#include "app.h"
#include "graphics/renderers/renderer.h"
#include "graphics/window.h"
#include "misc/animator.h"
#include "misc/io.h"
#include "misc/screen_manager.h"
#include "misc/utilities.h"
#include "screens/main_menu_screen.h"

namespace SingularityTrainer
{
const int resolution_x = 1920;
const int resolution_y = 1080;
const std::string window_title = "Singularity Trainer";
const int opengl_version_major = 4;
const int opengl_version_minor = 3;

int last_resolution_x = resolution_x;
int last_resolution_y = resolution_y;

void reset_imgui_style()
{
    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();

    // Padding/Spacing
    style.WindowPadding = {20, 20};
    style.FramePadding = {6, 3};
    style.ItemSpacing = {13, 10};
    style.ItemInnerSpacing = {7, 3};
    style.IndentSpacing = 20;

    // Sizing
    style.ScrollbarSize = 13;
    style.GrabMinSize = 13;

    // Borders
    style.WindowBorderSize = 0;
    style.ChildBorderSize = 0;
    style.FrameBorderSize = 0;
    style.PopupBorderSize = 0;
    style.TabBorderSize = 0;

    // Rounding
    style.ChildRounding = 1;
    style.FrameRounding = 1;
    style.GrabRounding = 1;
    style.PopupRounding = 1;
    style.ScrollbarRounding = 1;
    style.TabRounding = 1;
    style.WindowRounding = 1;

    // Alignment
    style.WindowTitleAlign = {0.5, 0.5};

    style.Colors[ImGuiCol_Text] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_TextDisabled] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_WindowBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_ChildBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_PopupBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_Border] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_BorderShadow] = glm_to_im(cl_base01);
    style.Colors[ImGuiCol_FrameBg] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_FrameBgHovered] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_FrameBgActive] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_TitleBg] = glm_to_im({cl_base3.r, cl_base3.g, cl_base3.b, 0.9});
    style.Colors[ImGuiCol_TitleBgCollapsed] = glm_to_im({cl_base3.r, cl_base3.g, cl_base3.b, 0.9});
    style.Colors[ImGuiCol_TitleBgActive] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_MenuBarBg] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ScrollbarBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_ScrollbarGrab] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_CheckMark] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_SliderGrab] = glm_to_im(cl_base01);
    style.Colors[ImGuiCol_SliderGrabActive] = glm_to_im(cl_base01);
    style.Colors[ImGuiCol_Button] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ButtonHovered] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_ButtonActive] = glm_to_im(cl_base0);
    style.Colors[ImGuiCol_Tab] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_TabHovered] = glm_to_im(cl_base1_l);
    style.Colors[ImGuiCol_TabActive] = glm_to_im(cl_base1_l);
    style.Colors[ImGuiCol_Header] = glm_to_im(cl_base1_l);
    style.Colors[ImGuiCol_HeaderHovered] = glm_to_im(cl_base1_l);
    style.Colors[ImGuiCol_HeaderActive] = glm_to_im(cl_base1_l);
    style.Colors[ImGuiCol_ResizeGrip] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ResizeGripHovered] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ResizeGripActive] = glm_to_im(cl_base01);
    style.Colors[ImGuiCol_PlotLines] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_PlotLinesHovered] = glm_to_im(cl_base01);
    style.Colors[ImGuiCol_PlotHistogram] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_PlotHistogramHovered] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_TextSelectedBg] = glm_to_im(cl_base02);
}

void init_imgui(const int opengl_version_major, const int opengl_version_minor, GLFWwindow *window)
{
    spdlog::debug("Initializing ImGui");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    std::stringstream string_stream;
    string_stream << "#version " << opengl_version_major << opengl_version_minor << "0 core";
    std::string x = string_stream.str();
    ImGui_ImplOpenGL3_Init(string_stream.str().c_str());

    ImFontConfig font_config;
    font_config.OversampleH = 3;
    font_config.OversampleV = 3;
    io.Fonts->ClearFonts();
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 16, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 24, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 32, &font_config);
    io.IniFilename = NULL;

    reset_imgui_style();
}

void error_callback(int error, const char *description)
{
    spdlog::error("GLFW error: [{}] {}", error, description);
}

void cursor_pos_callback(GLFWwindow *glfw_window, double x, double y)
{
    auto &io = static_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->get_io();
    double reversed_y = io.get_resolution().y - y;
    io.set_cursor_position(x, reversed_y);
}

void resize_window_callback(GLFWwindow *glfw_window, int x, int y)
{
    if (x == 0 || y == 0)
    {
        return;
    }
    spdlog::debug("Resizing window to {}x{}", x, y);
    glViewport(0, 0, x, y);

    // Scale windows (lossy)
    float window_relative_scale = static_cast<float>(x) / last_resolution_x;
    for (const auto &viewport : ImGui::GetCurrentContext()->Viewports)
    {
        ImGui::ScaleWindowsInViewport(viewport, window_relative_scale);
    }

    // Scale styles (not lossy)
    reset_imgui_style();
    float relative_scale = static_cast<float>(x) / resolution_x;
    ImGui::GetStyle().ScaleAllSizes(relative_scale);
    ImGui::GetIO().FontGlobalScale = relative_scale;

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
    window->get_renderer().resize(x, y);

    auto &io = window->get_io();
    io.set_resolution(x, y);

    last_resolution_x = x;
    last_resolution_y = y;
}

void key_callback(GLFWwindow *glfw_window, int key, int /*scancode*/, int actions, int /*mod*/)
{
    auto &io = static_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->get_io();
    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        if (actions == GLFW_PRESS)
        {
            io.press_key(key);
        }
        else if (actions == GLFW_RELEASE)
        {
            io.release_key(key);
        }
    }
}

void mouse_button_callback(GLFWwindow *glfw_window, int button, int action, int /*mods*/)
{
    auto &io = static_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->get_io();
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT &&
            action == GLFW_RELEASE &&
            !io.get_left_click())
        {
            io.left_click();
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT &&
                 action == GLFW_RELEASE &&
                 !io.get_right_click())
        {
            io.right_click();
        }
    }
}

App::App(Animator &animator,
         IO &io,
         Renderer &renderer,
         MainMenuScreenFactory &main_menu_screen_factory,
         ScreenManager &screen_manager,
         Window &window)
    : animator(animator),
      io(io),
      main_menu_screen_factory(main_menu_screen_factory),
      renderer(renderer),
      screen_manager(screen_manager),
      time(0),
      window(window)
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");
    glfwSetErrorCallback(error_callback);

    // Create window
    window.set_resize_callback(resize_window_callback);
    window.set_cursor_pos_callback(cursor_pos_callback);
    window.set_key_callback(key_callback);
    window.set_mouse_button_callback(mouse_button_callback);

    window.set_renderer(renderer);
    window.set_io(io);
    io.set_resolution(resolution_x, resolution_y);

    // Init cURL
    cURLpp::initialize();
}

int App::run(int argc, char *argv[])
{
    argh::parser args(argv);
    if (args[{"-t", "--test"}])
    {
        return run_tests(argc, argv, args);
    }

    init_imgui(opengl_version_major, opengl_version_minor, window.window);

    screen_manager.show_screen(main_menu_screen_factory.make());

    time = glfwGetTime();

    while (!window.should_close())
    {
        /*
         *  Input
         */
        glfwPollEvents();

        /*
         *  Update
         */
        double new_time = glfwGetTime();
        double delta_time = new_time - time;
        time = new_time;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        animator.update(delta_time);
        screen_manager.update(delta_time);
        if (screen_manager.stack_size() == 0)
        {
            window.close();
        }

        io.tick();

        /*
         *  Draw
         */
        if (screen_manager.stack_size() > 0)
        {
            renderer.begin();
            screen_manager.draw(renderer);
            renderer.render();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            window.swap_buffers();
        }
    }

    // Allow screens to perform cleanup
    screen_manager.exit();

    // Cleanup cURL
    cURLpp::terminate();

    return 0;
}

int App::run_tests(int argc, char *argv[], const argh::parser &args)
{
    if (!args["--with-logs"])
    {
        spdlog::set_level(spdlog::level::off);
    }
    doctest::Context context;

    context.setOption("order-by", "name");

    if (!args["--http"])
    {
        context.addFilter("test-case-exclude", "HttpClient");
    }

    context.applyCommandLine(argc, argv);

    return context.run();
}
}