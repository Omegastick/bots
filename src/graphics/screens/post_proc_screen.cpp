#include <vector>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>

#include "graphics/screens/post_proc_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/post_proc_layer.h"
#include "graphics/render_data.h"
#include "graphics/backend/texture.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
PostProcScreen::PostProcScreen(
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
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>();
    sprite->texture = "base_module";
    sprite->transform.set_scale(glm::vec2(100, 100));
    sprite->transform.set_position(glm::vec2(960, 540));

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("post_proc_test_1", "shaders/texture.vert", "shaders/post_proc_test.frag");
    resource_manager.load_shader("post_proc_test_2", "shaders/texture.vert", "shaders/post_proc_test.frag");

    post_proc_layer_1 = std::make_unique<PostProcLayer>(*resource_manager.shader_store.get("post_proc_test_1"));
    post_proc_layer_2 = std::make_unique<PostProcLayer>(*resource_manager.shader_store.get("post_proc_test_2"));
}

PostProcScreen::~PostProcScreen() {}

void PostProcScreen::update(double delta_time)
{
    display_test_dialog("Post processing test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->transform.rotate(1.f * delta_time);

    ImGui::Begin("Sprite position");
    float position[2]{sprite->transform.get_position().x, sprite->transform.get_position().y};
    ImGui::SliderFloat2("Position", position, 0, 1920);
    sprite->transform.set_position(glm::vec2(position[0], position[1]));
    ImGui::End();
}

void PostProcScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_view(projection);
    renderer.push_post_proc_layer(*post_proc_layer_1);
    renderer.push_post_proc_layer(*post_proc_layer_2);

    renderer.draw(*sprite);

    auto post_proc_shader_1 = resource_manager->shader_store.get("post_proc_test_1");
    post_proc_shader_1->set_uniform_2f("u_direction", glm::vec2(1, 1));
    post_proc_shader_1->set_uniform_2f("u_resolution", glm::vec2(renderer.get_width(), renderer.get_height()));

    auto post_proc_shader_2 = resource_manager->shader_store.get("post_proc_test_2");
    post_proc_shader_2->set_uniform_2f("u_direction", glm::vec2(-1, 1));
    post_proc_shader_2->set_uniform_2f("u_resolution", glm::vec2(renderer.get_width(), renderer.get_height()));
}
}