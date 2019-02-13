#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/sprite_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/shader.h"
#include "graphics/sprite.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"

namespace SingularityTrainer
{
SpriteTestScreen::SpriteTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>(resource_manager.texture_store.get("base_module"));
    sprite->set_origin(glm::vec2(50, 50));
    sprite->set_scale(glm::vec2(100, 100));
    sprite->set_position(glm::vec2(960, 540));

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
}

SpriteTestScreen::~SpriteTestScreen() {}

void SpriteTestScreen::update(const float delta_time)
{
    display_test_dialog("Sprite test", *screens, *screen_names, *screen_manager);
    sprite->rotate(1.f * delta_time);
}

void SpriteTestScreen::draw(bool lightweight)
{
    Renderer renderer;
    auto shader = resource_manager->shader_store.get("texture");

    glm::mat4 mvp = projection * sprite->get_transform();
    shader->set_uniform_mat4f("u_mvp", mvp);
    sprite->get_texture().bind();
    shader->set_uniform_1i("u_texture", 0);

    renderer.draw(sprite->get_vertex_array(), sprite->get_element_buffer(), *shader);
}
}