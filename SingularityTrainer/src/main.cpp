#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <argh.h>
#include <doctest.h>

#include "graphics/window.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "communicator.h"
#include "random.h"
#include "requests.h"
#include "screen_manager.h"
#include "resource_manager.h"
#include "screens/main_menu_screen.h"

using namespace SingularityTrainer;

const int resolution_x = 1920;
const int resolution_y = 1080;
const std::string window_title = "Singularity Trainer";
const int opengl_version_major = 4;
const int opengl_version_minor = 3;

void error_callback(int error, const char *description)
{
    spdlog::error("GLFW error: [{}] {}", error, description);
}

ImVec4 glm_to_im(const glm::vec4 &in)
{
    return {in.x, in.y, in.z, in.w};
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
    style.WindowBorderSize = 0;
    style.FrameBorderSize = 0;

    style.Colors[ImGuiCol_Text] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_TextDisabled] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_WindowBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_ChildBg] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_PopupBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_Border] = glm_to_im(cl_base03);
    style.Colors[ImGuiCol_BorderShadow] = glm_to_im(cl_base03);
    style.Colors[ImGuiCol_FrameBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_FrameBgHovered] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_FrameBgActive] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_TitleBg] = glm_to_im(cl_base3);
    style.Colors[ImGuiCol_TitleBgCollapsed] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_TitleBgActive] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_MenuBarBg] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ScrollbarBg] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ScrollbarGrab] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = glm_to_im(cl_base01);
    style.Colors[ImGuiCol_CheckMark] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_SliderGrab] = glm_to_im(cl_base00);
    style.Colors[ImGuiCol_SliderGrabActive] = glm_to_im(cl_base01);
    style.Colors[ImGuiCol_Button] = glm_to_im(cl_base1);
    style.Colors[ImGuiCol_ButtonHovered] = glm_to_im(cl_base2);
    style.Colors[ImGuiCol_ButtonActive] = glm_to_im(cl_base2);
    // style.Colors[ImGuiCol_Header] = glm_to_im();
    // style.Colors[ImGuiCol_HeaderHovered] glm_to_im();
    // // style.Colors[ImGuiCol_HeaderActive] = glm_to_im();
    // style.Colors[ImGuiCol_Separator] = glm_to_im();
    // style.Colors[ImGuiCol_SeparatorHovered] = glm_to_im();
    // style.Colors[ImGuiCol_SeparatorActive] = glm_to_im();
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
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 16, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 32, &font_config);

    reset_imgui_style();
}

void resize_window_callback(GLFWwindow *glfw_window, int x, int y)
{
    if (x == 0 || y == 0)
    {
        return;
    }
    spdlog::debug("Resizing window to {}x{}", x, y);
    glViewport(0, 0, x, y);

    reset_imgui_style();
    float relative_scale = (float)x / (float)resolution_x;
    ImGui::GetStyle().ScaleAllSizes(relative_scale);
    ImGui::GetIO().FontGlobalScale = relative_scale;

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
    window->get_renderer().resize(x, y);
}

int run_tests(int argc, const char *argv[], const argh::parser &args)
{
    if (!args["--with-logs"])
    {
        spdlog::set_level(spdlog::level::off);
    }
    doctest::Context context;

    context.setOption("order-by", "name");

    context.applyCommandLine(argc, argv);

    return context.run();
}

int main(int argc, const char *argv[])
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");
    glfwSetErrorCallback(error_callback);

    argh::parser args(argv);

    if (args[{"-t", "--test"}])
    {
        return run_tests(argc, argv, args);
    }

    // Create window
    Window window = Window(resolution_x, resolution_y, window_title, opengl_version_major, opengl_version_minor);
    window.set_resize_callback(resize_window_callback);

    ScreenManager screen_manager;
    ResourceManager resource_manager("assets/");
    Communicator communicator("tcp://127.0.0.1:10201");
    Random rng(1);
    Renderer renderer(1920, 1080, resource_manager);
    window.set_renderer(&renderer);

    std::shared_ptr<MainMenuScreen> test_screen = std::make_shared<MainMenuScreen>(resource_manager, communicator, screen_manager, rng);
    screen_manager.show_screen(test_screen);

    init_imgui(opengl_version_major, opengl_version_minor, window.window);

    float time = glfwGetTime();

    while (!window.should_close())
    {
        /*
         *  Input
         */
        glfwPollEvents();

        /*
         *  Update
         */
        float new_time = glfwGetTime();
        float delta_time = new_time - time;
        time = new_time;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        screen_manager.update(delta_time);

        /*
         *  Draw
         */
        screen_manager.draw(renderer);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swap_buffers();
    }

    // Allow screens to perform cleanup
    while (screen_manager.stack_size() > 0)
    {
        screen_manager.close_screen();
    }

    return 0;
}
