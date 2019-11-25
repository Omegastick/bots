#include <string>
#include <sstream>
#include <vector>

#ifdef BUILD_WITH_EASY_PROFILER
#include <easy/profiler.h>
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>

#include "graphics/window.h"
#include "graphics/renderers/line_renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/renderers/batched_sprite_renderer.h"
#include "graphics/renderers/text_renderer.h"
#include "graphics/renderers/vector_renderer.h"
#include "graphics/screens/screens.h"
#include "misc/animator.h"
#include "misc/io.h"
#include "misc/module_factory.h"
#include "misc/module_texture_store.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "misc/utilities.h"
#include "screens/iscreen.h"

using namespace SingularityTrainer;

const int resolution_x = 1920;
const int resolution_y = 1080;
const std::string window_title = "Graphics Playground";
const int opengl_version_major = 4;
const int opengl_version_minor = 3;

int last_resolution_x = resolution_x;
int last_resolution_y = resolution_y;

void error_callback(int error, const char *description)
{
    spdlog::error("GLFW error: [{}] {}", error, description);
}

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

    last_resolution_x = x;
    last_resolution_y = y;
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

int main(int /*argc*/, const char * /*argv*/ [])
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");
    glfwSetErrorCallback(error_callback);

#ifdef BUILD_WITH_EASY_PROFILER
    EASY_MAIN_THREAD;
    profiler::startListen();
#endif

    // Create window
    Window window = Window(resolution_x, resolution_y, window_title, opengl_version_major, opengl_version_minor);

    ResourceManager resource_manager("assets/");
    Animator animator;

    spdlog::debug("Initializing renderer");
    LineRenderer line_renderer(resource_manager);
    ParticleRenderer particle_renderer(100000, resource_manager);
    BatchedSpriteRenderer sprite_renderer(resource_manager);
    TextRenderer text_renderer(resource_manager);
    VectorRenderer vector_renderer;
    Renderer renderer(resolution_x,
                      resolution_y,
                      resource_manager,
                      sprite_renderer,
                      particle_renderer,
                      line_renderer,
                      text_renderer,
                      vector_renderer);
    window.set_renderer(renderer);
    window.set_resize_callback(resize_window_callback);

    Random rng(0);
    ModuleFactory module_factory(rng);

    LineRenderer module_line_renderer(resource_manager);
    ParticleRenderer module_particle_renderer(100000, resource_manager);
    BatchedSpriteRenderer module_sprite_renderer(resource_manager);
    TextRenderer module_text_renderer(resource_manager);
    VectorRenderer module_vector_renderer;
    Renderer module_renderer(resolution_x,
                             resolution_y,
                             resource_manager,
                             sprite_renderer,
                             particle_renderer,
                             line_renderer,
                             text_renderer,
                             vector_renderer);
    ModuleTextureStore module_texture_store(module_factory, std::move(module_renderer));

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
    spdlog::debug("Initializing line test");
    screens.push_back(std::make_shared<LineTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Line test");
    spdlog::debug("Initializing CRT test");
    screens.push_back(std::make_shared<CrtTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("CRT test");
    spdlog::debug("Initializing text test");
    screens.push_back(std::make_shared<TextTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Text test");
    spdlog::debug("Initializing animation test");
    screens.push_back(std::make_shared<AnimationTestScreen>(&screen_manager, resource_manager, animator, &screens, &screen_names));
    screen_names.push_back("Animation test");
    spdlog::debug("Initializing grid test");
    screens.push_back(std::make_shared<GridTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Grid test");
    spdlog::debug("Initializing distortion test");
    screens.push_back(std::make_shared<DistortionTestScreen>(&screen_manager, resource_manager, &screens, &screen_names));
    screen_names.push_back("Distortion test");
    spdlog::debug("Initializing plot test");
    screens.push_back(std::make_shared<PlotTestScreen>(screen_manager, screens, screen_names));
    screen_names.push_back("Plot test");
    spdlog::debug("Initializing bloom test");
    screens.push_back(std::make_shared<BloomTestScreen>(screen_manager,
                                                        resource_manager,
                                                        screens,
                                                        screen_names));
    screen_names.push_back("Bloom test");
    spdlog::debug("Initializing vector test");
    screens.push_back(std::make_shared<VectorTestScreen>(screen_manager,
                                                         resource_manager,
                                                         screens,
                                                         screen_names));
    screen_names.push_back("Vector test");
    spdlog::debug("Initializing vector test");
    screens.push_back(std::make_shared<TextureStoreTestScreen>(module_texture_store,
                                                               screen_manager,
                                                               resource_manager,
                                                               screens,
                                                               screen_names));
    screen_names.push_back("Texture store test");
    screen_manager.show_screen(screens[0]);

    // ImGui
    init_imgui(opengl_version_major, opengl_version_minor, window.window);

    double time = glfwGetTime();

    // Main loop
    while (!window.should_close())
    {
#ifdef BUILD_WITH_EASY_PROFILER
        EASY_BLOCK("Frame");
#endif
        // Time
        double new_time = glfwGetTime();
        double delta_time = new_time - time;
        time = new_time;

        // Input
        glfwPollEvents();

        // Update
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        animator.update(delta_time);
        screen_manager.update(delta_time);

        // Draw
        renderer.begin();
        screen_manager.draw(renderer, delta_time);
        renderer.render(time);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swap_buffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
