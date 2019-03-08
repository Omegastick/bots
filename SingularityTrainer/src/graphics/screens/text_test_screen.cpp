#include <vector>
#include <memory>
#include <string>
#include <fstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/text_test_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/screens/test_utils.h"
#include "graphics/stb_truetype.h"
#include "graphics/render_data.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/element_buffer.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"

namespace SingularityTrainer
{
TextTestScreen::TextTestScreen(
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
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-48", "fonts/Roboto-Regular.ttf", 48);

    text.text = "Hello world!";
    text.font = "roboto-48";
    text.set_position({500, 500});
    text.set_rotation(25);
}

TextTestScreen::~TextTestScreen() {}

void TextTestScreen::update(const float delta_time)
{
    display_test_dialog("Text test", *screens, *screen_names, delta_time, *screen_manager);
}

void TextTestScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    RenderData render_data;
    render_data.texts.push_back(text);
    renderer.draw(render_data, projection, 0);

    renderer.end();
}
}