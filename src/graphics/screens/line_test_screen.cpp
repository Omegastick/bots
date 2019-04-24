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
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"
#include "training/agents/test_agent.h"

namespace SingularityTrainer
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

    Line line_1;
    line_1.points.push_back({200, 200});
    line_1.colors.push_back({1.0, 0.0, 1.0, 1.0});
    line_1.widths.push_back(3);
    line_1.points.push_back({400, 500});
    line_1.colors.push_back({0.0, 1.0, 1.0, 1.0});
    line_1.widths.push_back(5);
    line_1.points.push_back({800, 700});
    line_1.colors.push_back({0.0, 1.0, 0.0, 1.0});
    line_1.widths.push_back(20);
    lines.push_back(line_1);

    Line line_2;
    line_2.points.push_back({1700, 900});
    line_2.colors.push_back({1.0, 1.0, 1.0, 1.0});
    line_2.widths.push_back(0);
    line_2.points.push_back({1600, 500});
    line_2.colors.push_back({1.0, 1.0, 1.0, 0.66});
    line_2.widths.push_back(5);
    line_2.points.push_back({1000, 1000});
    line_2.colors.push_back({1.0, 1.0, 1.0, 0.33});
    line_2.widths.push_back(5);
    line_2.points.push_back({700, 800});
    line_2.colors.push_back({1.0, 1.0, 1.0, 0.0});
    line_2.widths.push_back(5);
    lines.push_back(line_2);
}

LineTestScreen::~LineTestScreen() {}

void LineTestScreen::update(const float delta_time)
{
    display_test_dialog("Line test", *screens, *screen_names, delta_time, *screen_manager);
}

void LineTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    RenderData render_data;
    render_data.lines = lines;
    renderer.draw(render_data, projection, glfwGetTime());

    renderer.end();
}
}