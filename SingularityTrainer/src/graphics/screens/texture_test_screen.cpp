#include <vector>
#include <memory>
#include <string>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "iscreen.h"
#include "graphics/renderer.h"
#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/element_buffer.h"
#include "graphics/shader.h"
#include "graphics/imgui_utils.h"
#include "graphics/screens/texture_test_screen.h"
#include "graphics/screens/test_utils.h"

namespace SingularityTrainer
{
TextureTestScreen::TextureTestScreen(ScreenManager *screen_manager, std::vector<std::shared_ptr<IScreen>> *screens, std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)), rotation(0)
{
    vertex_array = std::make_unique<VertexArray>();

    float vertices[] = {
        -100, 100, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
        -100, -100, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0,
        100, -100, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0,
        100, 100, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0};

    vertex_buffer = std::make_unique<VertexBuffer>(vertices, 4 * 8 * sizeof(float));

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    element_buffer = std::make_unique<ElementBuffer>(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);

    texture = std::make_unique<Texture>("SingularityTrainer/assets/images/gun_module.png");
    texture->bind();

    shader = std::make_unique<Shader>("SingularityTrainer/assets/shaders/texture.vert", "SingularityTrainer/assets/shaders/texture.frag");
    shader->set_uniform_mat4f("u_mvp", projection);
    shader->set_uniform_1i("u_texture", 0);
}

TextureTestScreen::~TextureTestScreen() {}

void TextureTestScreen::update(const float delta_time)
{
    display_test_dialog("Texture test", *screens, *screen_names, *screen_manager);
    rotation += 1.f * delta_time;
}

void TextureTestScreen::draw(bool lightweight)
{
    Renderer renderer;

    glm::mat4 translate = glm::translate(glm::mat4(1.), glm::vec3(960, 540, 0));
    glm::mat4 rotate = glm::rotate(glm::mat4(1.), rotation, glm::vec3(0, 0, 1));
    glm::mat4 mvp = projection * translate * rotate;
    shader->set_uniform_mat4f("u_mvp", mvp);

    texture->bind();
    renderer.draw(*vertex_array, *element_buffer, *shader);
}
}