#include <vector>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "bloom_test_screen.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/post_processing/bloom_layer.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "graphics/screens/test_utils.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace ai
{
BloomTestScreen::BloomTestScreen(
    ScreenManager &screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> &screens,
    std::vector<std::string> &screen_names)
    : resource_manager(resource_manager),
      screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>();
    sprite->texture = "base_module";
    sprite->transform.set_scale(glm::vec2(200, 200));
    sprite->transform.set_position(glm::vec2(960, 540));

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("bloom", "shaders/highpass.vert", "shaders/highpass.frag");
    resource_manager.load_shader("blur", "shaders/blur.vert", "shaders/blur.frag");
    resource_manager.load_shader("combine", "shaders/blur.vert", "shaders/combine.frag");

    post_proc_layer = std::make_unique<BloomLayer>(resource_manager, 1920, 1080);
}

void BloomTestScreen::update(double delta_time)
{
    display_test_dialog("Bloom test", screens, screen_names, delta_time, screen_manager);
    sprite->transform.rotate(1.f * static_cast<float>(delta_time));

    ImGui::SetNextWindowSize({300, 100}, ImGuiCond_Once);
    ImGui::Begin("Sprite position");
    float position[2]{sprite->transform.get_position().x, sprite->transform.get_position().y};
    ImGui::SliderFloat("Position X", &position[0], 0, 1920);
    ImGui::SliderFloat("Position Y", &position[1], 0, 1080);
    sprite->transform.set_position(glm::vec2(position[0], position[1]));
    ImGui::End();
}

void BloomTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_view(projection);
    renderer.push_post_proc_layer(*post_proc_layer);

    renderer.draw(*sprite);
    const auto original_position = sprite->transform.get_position();
    sprite->transform.set_position(original_position + glm::vec2{100, 100});
    sprite->color = glm::vec4(0.5, 0.5, 0.5, 1.0);
    renderer.draw(*sprite);
    sprite->transform.set_position(original_position);
    sprite->color = glm::vec4(1.0, 1.0, 1.0, 1.0);
}
}