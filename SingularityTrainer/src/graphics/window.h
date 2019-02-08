#pragma once

#include <iostream>
#include <string>

#include "GLFW/glfw3.h"

namespace SingularityTrainer
{
class Window
{
  public:
    Window(int x, int y, std::string title, int major_opengl_version, int minor_opengl_version);
    ~Window();

    void swap_buffers();
    bool should_close();

    GLFWwindow *window;
};
}
