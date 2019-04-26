#include <memory>
#include <string>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "iscreen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
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

    texture = std::make_unique<Texture>("assets/images/gun_module.png");
    texture->bind();

    shader = std::make_unique<Shader>("assets/shaders/texture.vert", "assets/shaders/texture.frag");
    shader->set_uniform_mat4f("u_mvp", projection);
}

TextureTestScreen::~TextureTestScreen() {}

void TextureTestScreen::update(const double delta_time)
{
    display_test_dialog("Texture test", *screens, *screen_names, delta_time, *screen_manager);
    rotation += 1.f * delta_time;
}

void TextureTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    glm::mat4 mvp = glm::translate(projection, glm::vec3(960, 540, 0));
    mvp = glm::rotate(mvp, rotation, glm::vec3(0, 0, 1));
    shader->set_uniform_mat4f("u_mvp", mvp);
    texture->bind();
    shader->set_uniform_1i("u_texture", 0);

    renderer.draw(*vertex_array, *element_buffer, *shader);
}
}