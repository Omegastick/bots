#include <vector>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>

#include "distortion_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/sprite.h"
#include "graphics/post_proc_layer.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
DistortionTestScreen::DistortionTestScreen(
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
    sprite = std::make_unique<Sprite>("base_module");
    sprite->set_scale(glm::vec2(100, 100));
    sprite->set_origin(sprite->get_center());
    sprite->set_position(glm::vec2(960, 540));

    resource_manager.load_texture("distortion", "images/dudv.tiff");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("distortion", "shaders/texture.vert", "shaders/distortion.frag");

    post_proc_layer = std::make_unique<PostProcLayer>(
        resource_manager.shader_store.get("distortion").get());
}

DistortionTestScreen::~DistortionTestScreen() {}

void DistortionTestScreen::update(double delta_time)
{
    display_test_dialog("Distortion test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->rotate(1.f * delta_time);

    ImGui::Begin("Sprite position");
    float position[2]{sprite->get_position().x, sprite->get_position().y};
    ImGui::SliderFloat2("Position", position, 0, 1920);
    sprite->set_position(glm::vec2(position[0], position[1]));
    ImGui::End();
}

void DistortionTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(post_proc_layer.get());
    renderer.begin();

    renderer.draw(*sprite, projection);

    auto distortion_shader = resource_manager->shader_store.get("distortion");
    distortion_shader->set_uniform_1i("u_distortion", 1);

    auto distortion_textutre = resource_manager->texture_store.get("distortion");
    distortion_textutre->bind(1);

    renderer.end();
}
}