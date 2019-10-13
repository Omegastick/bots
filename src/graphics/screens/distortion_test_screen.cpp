#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <spdlog/spdlog.h>

#include "distortion_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{

const float resolution_x = 1920;
const float resolution_y = 1080;
const float edge_length = 100;
const int edge_count_x = std::ceil(resolution_x / edge_length) + 1;
const int edge_count_y = std::ceil(resolution_y / edge_length) + 1;

struct Vertex
{
    glm::vec2 position;
    glm::vec2 tex_coord;
    glm::vec4 color;
};

DistortionTestScreen::DistortionTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : resource_manager(resource_manager),
      screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      vertex_array(std::make_unique<VertexArray>())
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>("base_module");
    sprite->set_scale(glm::vec2(100, 100));
    sprite->set_origin(sprite->get_center());
    sprite->set_position(glm::vec2(960, 540));

    std::vector<Vertex> vertices = {
        {glm::vec2{0, 1080}, glm::vec2{0.0, 1.0}, glm::vec4{1.0, 1.0, 1.0, 1.0}},
        {glm::vec2{0, 0}, glm::vec2{0.0, 0.0}, glm::vec4{1.0, 1.0, 1.0, 1.0}},
        {glm::vec2{1920, 0}, glm::vec2{1.0, 0.0}, glm::vec4{1.0, 1.0, 1.0, 1.0}},
        {glm::vec2{1920, 1080}, glm::vec2{1.0, 1.0}, glm::vec4{1.0, 1.0, 1.0, 1.0}}};

    vertex_buffer = std::make_unique<VertexBuffer>(vertices.data(), 4 * sizeof(Vertex));

    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};
    element_buffer = std::make_unique<ElementBuffer>(indices.data(), 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);

    std::vector<float> pixels = {
        0, 0,
        0, 1,
        1, 0,
        1, 1};
    texture = std::make_unique<Texture>(2, 2, pixels.data());
}

void DistortionTestScreen::update(double delta_time)
{
    display_test_dialog("Distortion test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->rotate(1.f * delta_time);
}

void DistortionTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    vertex_array->bind();

    auto shader = resource_manager.shader_store.get("texture");
    shader->bind();

    texture->bind();
    shader->set_uniform_mat4f("u_mvp", projection);
    renderer.draw(*vertex_array, *element_buffer, *shader);

    renderer.end();
}
}