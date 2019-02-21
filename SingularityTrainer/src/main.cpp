#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "graphics/window.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "communicator.h"
#include "random.h"
#include "requests.h"
#include "screen_manager.h"
#include "screens/target_env_screen.h"

using namespace SingularityTrainer;

const int resolution_x = 1920;
const int resolution_y = 1080;
const std::string window_title = "Singularity Trainer";
const int opengl_version_major = 4;
const int opengl_version_minor = 3;

void error_callback(int error, const char *description)
{
    spdlog::error("GLFW error: [{}] {}", error, description);
}

void resize_window_callback(GLFWwindow *glfw_window, int x, int y)
{
    spdlog::debug("Resizing window to {}x{}", x, y);
    glViewport(0, 0, x, y);

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
    window->get_renderer().resize(x, y);
}

int main(int argc, const char *argv[])
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");
    glfwSetErrorCallback(error_callback);

    // Create window
    Window window = Window(resolution_x, resolution_y, window_title, opengl_version_major, opengl_version_minor);

    ScreenManager screen_manager;
    ResourceManager resource_manager("SingularityTrainer/assets/");
    Communicator communicator("tcp://127.0.0.1:10201");
    Random rng(1);
    Renderer renderer(1920, 1080, resource_manager);

    std::shared_ptr<TargetEnvScreen> test_screen = std::make_shared<TargetEnvScreen>(resource_manager, &communicator, &rng, 7);
    screen_manager.show_screen(test_screen);

    float time = glfwGetTime();

    while (!window.should_close())
    {
        /*
         *  Input
         */
        glfwPollEvents();

        /*
         *  Update
         */
        float new_time = glfwGetTime();
        float delta_time = new_time - time;
        time = new_time;

        screen_manager.update(delta_time);

        /*
         *  Draw
         */
        screen_manager.draw(renderer);
        window.swap_buffers();
    }

    return 0;
}
