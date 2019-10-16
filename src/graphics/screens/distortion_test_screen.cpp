#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
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
const int width = 192;
const int height = 108;

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
      vertex_array(std::make_unique<VertexArray>()),
      spring_mesh(width, height)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>("base_module");
    sprite->transform.set_scale(glm::vec2(100, 100));
    sprite->transform.set_position(glm::vec2(960, 540));

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

    resource_manager.load_shader("distortion",
                                 "shaders/distortion.vert",
                                 "shaders/distortion.frag");
    post_proc_layer = std::make_unique<PostProcLayer>(
        *resource_manager.shader_store.get("distortion"));
}

void DistortionTestScreen::update(double delta_time)
{
    display_test_dialog("Distortion test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->transform.rotate(1.f * delta_time);

    if (ImGui::IsKeyPressed(GLFW_KEY_SPACE))
    {
        // glm::vec2 target_point = glm::linearRand(glm::vec2{0, 0},
        //  glm::vec2{width, height});
        spring_mesh.apply_explosive_force({96, 54}, 2, 0.5);
    }

    spring_mesh.update();
}

void DistortionTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(post_proc_layer.get());

    sprite->transform.set_position({860, 440});
    renderer.draw(*sprite, projection);

    sprite->transform.set_position({960, 540});
    renderer.draw(*sprite, projection);

    sprite->transform.set_position({1060, 640});
    renderer.draw(*sprite, projection);

    sprite->transform.set_position({1060, 440});
    renderer.draw(*sprite, projection);

    sprite->transform.set_position({860, 640});
    renderer.draw(*sprite, projection);

    vertex_array->bind();

    auto shader = resource_manager.shader_store.get("distortion");
    shader->bind();

    std::vector<float> pixels(width * height * 2);
    auto &offsets = spring_mesh.get_offsets();
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        pixels[i * 2] = offsets[i].x;
        pixels[i * 2 + 1] = offsets[i].y;
    }
    texture = std::make_unique<Texture>(width, height, pixels.data());

    texture->bind(1);
    shader->set_uniform_1i("u_distortion", 1);
}
}