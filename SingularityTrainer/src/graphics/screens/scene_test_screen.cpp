#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/scene_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"
#include "training/modules/base_module.h"

namespace SingularityTrainer
{
SceneTestScreen::SceneTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
}

SceneTestScreen::~SceneTestScreen() {}

void SceneTestScreen::update(const float delta_time)
{
    display_test_dialog("Scene test", *screens, *screen_names, delta_time, *screen_manager);
}

void SceneTestScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

        renderer.end();
}
}