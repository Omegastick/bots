#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/line_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace ai
{
LineTestScreen::LineTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    this->resource_manager = &resource_manager;

    lines.emplace_back(Line{{1800.f, 200.f},
                            {400.f, 700.f},
                            cl_white,
                            20.f});
    lines.emplace_back(Line{{100.f, 200.f},
                            {200.f, 400.f},
                            cl_red,
                            20.f});
}

void LineTestScreen::update(double delta_time)
{
    display_test_dialog("Line test", *screens, *screen_names, delta_time, *screen_manager);
}

void LineTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_view(projection);
    for (const auto &line : lines)
    {
        renderer.draw(line);
    }
}
}