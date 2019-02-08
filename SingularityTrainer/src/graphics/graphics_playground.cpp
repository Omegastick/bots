#include <iostream>
#include <string>

#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "spdlog/spdlog.h"

#include "graphics/window.h"

using namespace SingularityTrainer;

const int resolution_x = 1280;
const int resolution_y = 720;
const std::string window_title = "Graphics Playground";
const int opengl_version_major = 4;
const int opengl_version_minor = 3;

void error_callback(int error, const char *description)
{
    spdlog::error("GLFW error: [{}] {}", error, description);
}

int main(int argc, const char *argv[])
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %5l] %v%$");
    glfwSetErrorCallback(error_callback);

    // Create window
    Window window = Window(resolution_x, resolution_y, window_title, opengl_version_major, opengl_version_minor);

    glClearColor(0.3, 0.4, 0.8, 1.0);

    // Main loop
    while (!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
