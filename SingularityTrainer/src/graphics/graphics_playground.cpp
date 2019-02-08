#include <iostream>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

#include "graphics/window.h"

using namespace SingularityTrainer;

void error_callback(int error, const char *description)
{
    std::cerr << "GLFW error: [" << error << "] " << description << "\n";
}

GLFWwindow *init_window()
{
    if (!glfwInit())
    {
        std::cerr << "Unable to initialize GLFW\n";
        throw std::exception();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Graphics Playground", nullptr, nullptr);
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

    return window;
}

int main(int argc, const char *argv[])
{
    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = init_window();

    std::cout << glGetString(GL_VERSION) << "\n";

    glClearColor(0.3, 0.4, 0.8, 1.0);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
