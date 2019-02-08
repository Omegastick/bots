#include <iostream>
#include <string>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

#include "graphics/window.h"

namespace SingularityTrainer
{
Window::Window(int x, int y, std::string title, int opengl_major_version, int opengl_minor_version)
{
    if (!glfwInit())
    {
        std::cerr << "Unable to initialize GLFW\n";
        throw std::exception();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(x, y, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Unable to create window\n";
        glfwTerminate();
        throw std::exception();
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL())
    {
        std::cerr << "Unable to initialize GLAD\n";
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