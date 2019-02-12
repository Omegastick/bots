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
#include "graphics/screens/test_utils.h"
#include "graphics/screens/quad_screen.h"

namespace SingularityTrainer
{
QuadScreen::QuadScreen(ScreenManager *screen_manager, std::vector<std::shared_ptr<IScreen>> *screens, std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    vertex_array = std::make_unique<VertexArray>();

    float vertices[] = {
        640, 720, 1.0, 0.0, 0.0, 1.0,
        640, 360, 0.0, 1.0, 0.0, 1.0,
        1280, 360, 0.0, 0.0, 1.0, 1.0,
        1280, 720, 1.0, 0.0, 1.0, 1.0};

    vertex_buffer = std::make_unique<VertexBuffer>(vertices, 4 * 6 * sizeof(float));

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    element_buffer = std::make_unique<ElementBuffer>(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);

    shader = std::make_unique<Shader>("SingularityTrainer/assets/shaders/default.vert", "SingularityTrainer/assets/shaders/default.frag");
    shader->set_uniform_mat4f("u_mvp", projection);
    shader->bind();
}

QuadScreen::~QuadScreen() {}

void QuadScreen::update(const float delta_time)
{
    display_test_dialog("Quad test", *screens, *screen_names, *screen_manager);
}

void QuadScreen::draw(bool lightweight)
{
    Renderer renderer;
    renderer.draw(*vertex_array, *element_buffer, *shader);
}
}