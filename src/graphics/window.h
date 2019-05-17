#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
class Window
{
  private:
    Renderer *renderer;

  public:
    Window(int x, int y, std::string title, int major_opengl_version, int minor_opengl_version);
    ~Window();

    void swap_buffers();
    bool should_close();
    void set_cursor_pos_callback(void (*callback)(GLFWwindow *, double, double));
    void set_resize_callback(void (*callback)(GLFWwindow *, int, int));
    void set_renderer(Renderer *renderer);

    inline Renderer &get_renderer() const { return *renderer; }

    GLFWwindow *window;
};
}
