#include <iostream>
#include <string>

#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "spdlog/spdlog.h"

#include "graphics/window.h"

namespace SingularityTrainer
{
Window::Window(int x, int y, std::string title, int opengl_major_version, int opengl_minor_version)
{
    spdlog::debug("Creating {}x{} window with OpenGL version {}.{}", x, y, opengl_major_version, opengl_minor_version);
    if (!glfwInit())
    {
        spdlog::error("Unable to initialize GLFW");
        throw std::exception();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(x, y, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        spdlog::error("Unable to create window");
        glfwTerminate();
        throw std::exception();
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL())
    {
        spdlog::error("Unable to initialize GLAD");
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::exception();
    }
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::swap_buffers()
{
    glfwSwapBuffers(window);
}

bool Window::should_close()
{
    return glfwWindowShouldClose(window);
}
}