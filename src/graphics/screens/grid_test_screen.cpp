#include <vector>
#include <memory>
#include <string>

#include <easy/profiler.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "grid_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/backend/shader.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "graphics/sprite.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "misc/spring_mesh.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
const int width = 192;
const int height = 108;
const float size = 1000;

GridTestScreen::GridTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      spring_mesh(width, height),
      sprite_renderer(resource_manager)
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("bullet", "images/bullet.png");
    sprite = std::make_unique<Sprite>("bullet");
    sprite->set_scale(glm::vec2(3, 3));
}

void GridTestScreen::update(double delta_time)
{
    display_test_dialog("Grid test", *screens, *screen_names, delta_time, *screen_manager);

    if (ImGui::IsKeyPressed(GLFW_KEY_SPACE))
    {
        glm::vec2 target_point = glm::linearRand(glm::vec2{0, 0},
                                                 glm::vec2{width, height});
        spring_mesh.apply_explosive_force(target_point, 30, 100);
    }

    spring_mesh.update();
}

void GridTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    std::vector<glm::mat4> transforms;
    transforms.reserve(width * height);

    auto vertices = spring_mesh.get_vertices(1920, 1080);

    for (const auto &vertex : vertices)
    {
        sprite->set_position({vertex.x, vertex.y});
        transforms.push_back(sprite->get_transform());
    }

    sprite_renderer.draw(sprite->get_texture(), transforms, projection);
    renderer.end();
}
}