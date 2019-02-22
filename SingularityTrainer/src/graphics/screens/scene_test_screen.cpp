#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <Box2D/Box2D.h>

#include "graphics/screens/scene_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"
#include "training/agents/test_agent.h"

namespace SingularityTrainer
{
SceneTestScreen::SceneTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(-9.6f, 9.6f, -5.4f, 5.4f)),
      random(0),
      elapsed_time(0)
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");

    b2_world = std::make_unique<b2World>(b2Vec2(0, 0));
    agent = std::make_unique<TestAgent>(resource_manager, *b2_world, &random);
    agent->rigid_body->body->ApplyAngularImpulse(1, true);
}

SceneTestScreen::~SceneTestScreen() {}

void SceneTestScreen::update(const float delta_time)
{
    elapsed_time += delta_time;
    display_test_dialog("Scene test", *screens, *screen_names, delta_time, *screen_manager);
    b2_world->Step(delta_time, 6, 4);
    if (elapsed_time >= 1.f / 10.f)
    {
        elapsed_time = 0;
        agent->act({1, 1, 1, 1});
        agent->get_observation();
    }
}

void SceneTestScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    auto render_data = agent->get_render_data();
    renderer.draw(render_data, projection, glfwGetTime());

    renderer.end();
}
}