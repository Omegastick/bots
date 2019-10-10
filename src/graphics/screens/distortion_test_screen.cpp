#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "distortion_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/backend/shader.h"
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

struct SpriteVertex
{
    glm::vec2 position;
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

    resource_manager.load_shader("default", "shaders/default.vert", "shaders/default.frag");

    const int total_vertices = edge_count_x * edge_count_y;
    std::vector<SpriteVertex> vertices;
    for (float x = 0; x < resolution_x + edge_length; x += edge_length)
    {
        for (float y = 0; y < resolution_y + edge_length; y += edge_length)
        {
            vertices.push_back({glm::vec2{x, y},
                                glm::vec4{x / resolution_x, y / resolution_y, 1, 1}});
        }
    }
    vertex_buffer = std::make_unique<VertexBuffer>(vertices.data(),
                                                   total_vertices * sizeof(SpriteVertex));

    unsigned int sprite_indices[] = {1, 0, edge_count_y,
                                     edge_count_y, edge_count_y + 1, 1};
    std::vector<unsigned int> vertex_indices;
    for (unsigned int x = 0; x < edge_count_x - 1; ++x)
    {
        for (unsigned int y = 0; y < edge_count_y - 1; ++y)
        {
            unsigned int column_index = x * edge_count_y;
            unsigned int column_index_1 = (x + 1) * edge_count_y;
            vertex_indices.push_back(column_index + y + 1);
            vertex_indices.push_back(column_index + y);
            vertex_indices.push_back(column_index_1 + y);
            vertex_indices.push_back(column_index_1 + y);
            vertex_indices.push_back(column_index_1 + y + 1);
            vertex_indices.push_back(column_index + y + 1);
        }
    }
    element_buffer = std::make_unique<ElementBuffer>(vertex_indices.data(),
                                                     vertex_indices.size());

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);
}

DistortionTestScreen::~DistortionTestScreen() {}

void DistortionTestScreen::update(double delta_time)
{
    display_test_dialog("Distortion test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->rotate(1.f * delta_time);
}

void DistortionTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    vertex_array->bind();

    auto shader = resource_manager.shader_store.get("default");
    shader->bind();

    shader->set_uniform_mat4f("u_mvp", projection);
    renderer.draw(*vertex_array, *element_buffer, *shader);

    renderer.end();
}
}