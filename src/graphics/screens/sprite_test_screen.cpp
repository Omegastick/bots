#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/sprite_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/backend/shader.h"
#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

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
    sprite = std::make_unique<Sprite>("base_module");
    sprite->set_scale(glm::vec2(100, 100));
    sprite->set_position(glm::vec2(960, 540));

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
}

SpriteTestScreen::~SpriteTestScreen() {}

void SpriteTestScreen::update(double delta_time)
{
    display_test_dialog("Sprite test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->rotate(1.f * delta_time);
}

void SpriteTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();
    renderer.draw(*sprite, projection);
    renderer.end();
}
}