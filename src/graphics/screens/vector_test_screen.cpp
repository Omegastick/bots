#include <vector>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "vector_test_screen.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "graphics/screens/test_utils.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
VectorTestScreen::VectorTestScreen(
    ScreenManager &screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> &screens,
    std::vector<std::string> &screen_names)
    : resource_manager(resource_manager),
      screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(-19.2f, 19.2f, -10.8f, 10.8f)),
      rotation(0) {}

void VectorTestScreen::update(double delta_time)
{
    display_test_dialog("Vector test", screens, screen_names, delta_time, screen_manager);

    rotation += static_cast<float>(delta_time) * 0.1f;
}

void VectorTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    vector_renderer.begin_frame({renderer.get_width(), renderer.get_height()});

    renderer.set_view(projection);
    vector_renderer.set_view(projection);

    for (float x = -4.8f; x <= 4.8f; x += 19.2f / 20.f)
    {
        for (float y = -2.7f; y <= 2.7f; y += 10.8f / 10.f)
        {
            Rectangle rectangle{
                {0.5f, 0.5f, 0.5f, 0.5f},
                cl_white,
                0.2f,
                Transform()};
            rectangle.transform.set_position({x, y});
            rectangle.transform.resize({1.f, 1.f});
            rectangle.transform.rotate(rotation);
            vector_renderer.draw(rectangle);

            Circle circle{
                0.2f,
                cl_white,
                {0, 0, 0, 0},
                0.f,
                Transform()};
            circle.transform.set_position({x, y});
            circle.transform.rotate(rotation);
            vector_renderer.draw(circle);
        }
    }

    vector_renderer.end_frame();
}
}