#include <string>
#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "graphics/window.h"
#include "graphics/shader.h"
#include "graphics/vertex_buffer.h"
#include "graphics/element_buffer.h"
#include "graphics/vertex_array.h"
#include "graphics/renderer.h"

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

void resize_window_callback(GLFWwindow *window, int x, int y)
{
    glViewport(0, 0, x, y);

    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();
    float relative_scale = (float)x / (float)resolution_x;
    style.ScaleAllSizes(relative_scale);
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = relative_scale;
}

void init_imgui(const int opengl_version_major, const int opengl_version_minor, GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    std::stringstream string_stream;
    string_stream << "#version " << opengl_version_major << opengl_version_minor << "0 core";
    std::string x = string_stream.str();
    ImGui_ImplOpenGL3_Init(string_stream.str().c_str());

    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();

    ImFontConfig font_config;
    font_config.OversampleH = 3;
    font_config.OversampleV = 3;
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->ClearFonts();
    io.Fonts->AddFontFromFileTTF("SingularityTrainer/assets/fonts/Roboto-Regular.ttf", 13, &font_config);

    io.IniFilename = NULL;
}

int main(int argc, const char *argv[])
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %5l] %v%$");
    glfwSetErrorCallback(error_callback);

    // Create window
    Window window = Window(resolution_x, resolution_y, window_title, opengl_version_major, opengl_version_minor);
    window.set_resize_callback(resize_window_callback);

    // OpenGL stuff
    VertexArray vertex_array;

    float vertices[] = {
        -0.5, 0.5, 1.0, 0.0, 0.0, 1.0,
        -0.5, -0.5, 0.0, 1.0, 0.0, 1.0,
        0.5, -0.5, 0.0, 0.0, 1.0, 1.0,
        0.5, 0.5, 1.0, 0.0, 1.0, 1.0};

    VertexBuffer vertex_buffer(vertices, 4 * 6 * sizeof(float));

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    ElementBuffer element_buffer(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array.add_buffer(vertex_buffer, layout);

    Shader shader("SingularityTrainer/assets/shaders/default.vert", "SingularityTrainer/assets/shaders/default.frag");
    shader.bind();

    Renderer renderer;

    // ImGui
    init_imgui(opengl_version_major, opengl_version_minor, window.window);
    bool show_demo_window = true;

    // Main loop
    while (!window.should_close())
    {
        // Input
        glfwPollEvents();

        // Update
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // Draw
        renderer.clear();

        vertex_array.bind();
        renderer.draw(vertex_array, element_buffer, shader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swap_buffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
