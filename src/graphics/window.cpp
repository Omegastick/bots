#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "graphics/renderers/renderer.h"
#include "graphics/window.h"

namespace SingularityTrainer
{
void glDebugOutput(unsigned int source,
                   unsigned int type,
                   unsigned int id,
                   unsigned int severity,
                   int /*length*/,
                   const char *message,
                   const void * /*userParam*/)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::stringstream error_message;
    error_message << "OpenGL: ";

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        error_message << "API - ";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        error_message << "Window system - ";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        error_message << "Shader compiler - ";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        error_message << "Third party - ";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        error_message << "Application - ";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        error_message << "Other - ";
        break;
    }
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        error_message << "Error - ";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        error_message << "Deprecated behaviour - ";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        error_message << "Undefined behaviour - ";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        error_message << "Portability - ";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        error_message << "Performance - ";
        break;
    case GL_DEBUG_TYPE_MARKER:
        error_message << "Marker - ";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        error_message << "Push group - ";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        error_message << "Pop group - ";
        break;
    case GL_DEBUG_TYPE_OTHER:
        error_message << "Other - ";
        break;
    }

    error_message << message;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        spdlog::error(error_message.str());
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        spdlog::error(error_message.str());
        break;
    case GL_DEBUG_SEVERITY_LOW:
        spdlog::warn(error_message.str());
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        spdlog::info(error_message.str());
        break;
    }
}

Window::Window(int x, int y, std::string title, int opengl_major_version, int opengl_minor_version)
{
    spdlog::debug("Creating {}x{} window with target OpenGL version {}.{}", x, y, opengl_major_version, opengl_minor_version);
    if (!glfwInit())
    {
        spdlog::error("Unable to initialize GLFW");
        throw std::exception();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

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

    // Debug
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROC)glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    // glfwSwapInterval(0);
    glfwSetWindowUserPointer(window, this);

    spdlog::debug("Actual OpenGL version: {}", glGetString(GL_VERSION));
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::close()
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Window::swap_buffers()
{
    glfwSwapBuffers(window);
}

bool Window::should_close()
{
    return glfwWindowShouldClose(window);
}

void Window::set_cursor_pos_callback(void (*callback)(GLFWwindow *, double, double))
{
    glfwSetCursorPosCallback(window, callback);
}

void Window::set_key_callback(void (*callback)(GLFWwindow *, int key, int scancode, int actions, int mod))
{
    glfwSetKeyCallback(window, callback);
}

void Window::set_mouse_button_callback(void (*callback)(GLFWwindow *, int, int, int))
{
    glfwSetMouseButtonCallback(window, callback);
}

void Window::set_resize_callback(void (*callback)(GLFWwindow *, int, int))
{
    glfwSetWindowSizeCallback(window, callback);
}

void Window::set_renderer(Renderer &renderer)
{
    this->renderer = &renderer;
}

void Window::set_io(IO &io)
{
    this->io = &io;
}
}