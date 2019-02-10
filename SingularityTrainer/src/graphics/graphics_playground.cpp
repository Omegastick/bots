#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

#include "graphics/window.h"
#include "graphics/shader.h"
#include "graphics/vertex_buffer.h"
#include "graphics/element_buffer.h"

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

    // OpenGL stuff
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));

    Shader shader("SingularityTrainer/assets/shaders/default.vert", "SingularityTrainer/assets/shaders/default.frag");
    shader.bind();

    // Main loop
    while (!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, element_buffer.get_count(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
