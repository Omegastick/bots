#include <vector>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>

#include "graphics/screens/crt_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/sprite.h"
#include "graphics/post_proc_layer.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"

namespace SingularityTrainer
{
CrtTestScreen::CrtTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>("base_module");
    sprite->set_scale(glm::vec2(100, 100));
    sprite->set_origin(sprite->get_center());
    sprite->set_position(glm::vec2(960, 540));

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");

    post_proc_layer = std::make_unique<PostProcLayer>(resource_manager.shader_store.get("crt").get());
}

CrtTestScreen::~CrtTestScreen() {}

void CrtTestScreen::update(const double delta_time)
{
    display_test_dialog("CRT test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->rotate(1.f * delta_time);

    ImGui::Begin("Sprite position");
    float position[2]{sprite->get_position().x, sprite->get_position().y};
    ImGui::SliderFloat2("Position", position, 0, 1920);
    sprite->set_position(glm::vec2(position[0], position[1]));
    ImGui::End();
}

void CrtTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(post_proc_layer.get());
    renderer.begin();

    renderer.draw(*sprite, projection);

    auto crt_shader = resource_manager->shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", glm::vec2(renderer.get_width(), renderer.get_height()));
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 1);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.03);

    renderer.end();
}
}