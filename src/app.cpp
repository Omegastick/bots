#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <argh.h>
#include <curl/curl.h>
#include <doctest.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>

#include "app.h"
#include "audio/audio_engine.h"
#include "graphics/backend/shader.h"
#include "graphics/post_processing/bloom_layer.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/window.h"
#include "misc/animator.h"
#include "misc/io.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "misc/utilities.h"
#include "screens/main_menu_screen.h"
#include "ui/background.h"

namespace ai
{
const int resolution_x = 1920;
const int resolution_y = 1080;
const std::string window_title = "AI: Artificial Insentience";
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
    style.FramePadding = {3, 3};
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

    style.AntiAliasedLines = true;

    style.Colors[ImGuiCol_Text] = cl_base00;
    style.Colors[ImGuiCol_TextDisabled] = cl_base1;
    style.Colors[ImGuiCol_WindowBg] = cl_base3;
    style.Colors[ImGuiCol_ChildBg] = cl_base3;
    style.Colors[ImGuiCol_PopupBg] = cl_base3;
    style.Colors[ImGuiCol_Border] = cl_base00;
    style.Colors[ImGuiCol_BorderShadow] = cl_base01;
    style.Colors[ImGuiCol_FrameBg] = cl_base2;
    style.Colors[ImGuiCol_FrameBgHovered] = cl_base2;
    style.Colors[ImGuiCol_FrameBgActive] = cl_base2;
    style.Colors[ImGuiCol_TitleBg] = set_alpha(cl_base3, 0.9f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = set_alpha(cl_base3, 0.9f);
    style.Colors[ImGuiCol_TitleBgActive] = cl_base3;
    style.Colors[ImGuiCol_MenuBarBg] = cl_base2;
    style.Colors[ImGuiCol_ScrollbarBg] = cl_base3;
    style.Colors[ImGuiCol_ScrollbarGrab] = cl_base2;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = cl_base1;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = cl_base1;
    style.Colors[ImGuiCol_CheckMark] = cl_base1;
    style.Colors[ImGuiCol_SliderGrab] = cl_base01;
    style.Colors[ImGuiCol_SliderGrabActive] = cl_base01;
    style.Colors[ImGuiCol_Button] = cl_base2;
    style.Colors[ImGuiCol_ButtonHovered] = cl_base1;
    style.Colors[ImGuiCol_ButtonActive] = cl_base0;
    style.Colors[ImGuiCol_Tab] = cl_base2;
    style.Colors[ImGuiCol_TabHovered] = cl_base1_l;
    style.Colors[ImGuiCol_TabActive] = cl_base1_l;
    style.Colors[ImGuiCol_Header] = cl_base1_l;
    style.Colors[ImGuiCol_HeaderHovered] = cl_base1_l;
    style.Colors[ImGuiCol_HeaderActive] = cl_base1_l;
    style.Colors[ImGuiCol_ResizeGrip] = cl_base2;
    style.Colors[ImGuiCol_ResizeGripHovered] = cl_base2;
    style.Colors[ImGuiCol_ResizeGripActive] = cl_base01;
    style.Colors[ImGuiCol_PlotLines] = cl_base00;
    style.Colors[ImGuiCol_PlotLinesHovered] = cl_base01;
    style.Colors[ImGuiCol_PlotHistogram] = cl_base00;
    style.Colors[ImGuiCol_PlotHistogramHovered] = cl_base00;
    style.Colors[ImGuiCol_TextSelectedBg] = cl_base02;
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
    io.Fonts->AddFontFromFileTTF("assets/fonts/SourceCodePro-Light.ttf", 16, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/SourceCodePro-Light.ttf", 24, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/SourceCodePro-Light.ttf", 32, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/SourceCodePro-Regular.ttf", 16, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/SourceCodePro-Regular.ttf", 24, &font_config);
    io.Fonts->AddFontFromFileTTF("assets/fonts/SourceCodePro-Regular.ttf", 32, &font_config);
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
         AudioEngine &audio_engine,
         Background &background,
         IO &io,
         Renderer &renderer,
         ResourceManager &resource_manager,
         MainMenuScreenFactory &main_menu_screen_factory,
         ScreenManager &screen_manager,
         Window &window)
    : animator(animator),
      audio_engine(audio_engine),
      background(background),
      io(io),
      main_menu_screen_factory(main_menu_screen_factory),
      renderer(renderer),
      resource_manager(resource_manager),
      screen_manager(screen_manager),
      time(0),
      window(window)
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");

    // Init cURL
    curl_global_init(CURL_GLOBAL_ALL);
}

int App::run(int argc, char *argv[])
{
    argh::parser args(argv);
    if (args[{"-t", "--test"}])
    {
        return run_tests(argc, argv, args);
    }

    window.init();
    glfwSetErrorCallback(error_callback);
    renderer.init();

    // Create window
    window.set_resize_callback(resize_window_callback);
    window.set_cursor_pos_callback(cursor_pos_callback);
    window.set_key_callback(key_callback);
    window.set_mouse_button_callback(mouse_button_callback);

    window.set_renderer(renderer);
    window.set_io(io);
    io.set_resolution(resolution_x, resolution_y);

    // Initialize post processing
    resource_manager.load_shader("bloom", "shaders/highpass.vert", "shaders/highpass.frag");
    resource_manager.load_shader("blur", "shaders/blur.vert", "shaders/blur.frag");
    resource_manager.load_shader("combine", "shaders/blur.vert", "shaders/combine.frag");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("tone_map", "shaders/tone_map.vert", "shaders/tone_map.frag");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    bloom_post_proc_layer = std::make_unique<BloomLayer>(resource_manager,
                                                         io.get_resolution().x,
                                                         io.get_resolution().y);
    crt_post_proc_layer = std::make_unique<PostProcLayer>(
        *resource_manager.shader_store.get("crt"),
        io.get_resolution().x,
        io.get_resolution().y);
    tone_map_post_proc_layer = std::make_unique<PostProcLayer>(
        *resource_manager.shader_store.get("crt"),
        io.get_resolution().x,
        io.get_resolution().y);

    // Load sounds
    resource_manager.load_audio_source("hit_wall", "audio/hit_wall.wav");
    resource_manager.load_audio_source("hit_body", "audio/hit_body.wav");
    resource_manager.load_audio_source("fire", "audio/fire.wav");
    resource_manager.audio_source_store.get("fire")->setVolume(0.5f);
    resource_manager.load_audio_source("chord", "audio/chord.wav");
    resource_manager.load_audio_source("note", "audio/note.wav");

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
        audio_engine.update(delta_time);
        background.update(delta_time);
        screen_manager.update(delta_time);
        if (screen_manager.stack_size() == 0)
        {
            window.close();
        }

        io.tick();

        if (screen_manager.stack_size() == 0)
        {
            break;
        }

        /*
         *  Draw
         */
        renderer.begin();

        background.draw(renderer);
        screen_manager.draw(renderer);

        renderer.push_post_proc_layer(*crt_post_proc_layer);
        renderer.push_post_proc_layer(*bloom_post_proc_layer);
        renderer.push_post_proc_layer(*tone_map_post_proc_layer);
        auto crt_shader = resource_manager.shader_store.get("crt");
        crt_shader->set_uniform_2f("u_resolution",
                                   {renderer.get_width(), renderer.get_height()});

        renderer.render(time);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swap_buffers();
    }

    // Allow screens to perform cleanup
    screen_manager.exit();

    // Cleanup cURL
    curl_global_cleanup();

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