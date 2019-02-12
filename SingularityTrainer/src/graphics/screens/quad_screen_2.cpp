#include <vector>
#include <memory>
#include <string>

#include <imgui.h>

#include "iscreen.h"
#include "graphics/renderer.h"
#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/element_buffer.h"
#include "graphics/shader.h"
#include "graphics/imgui_utils.h"
#include "quad_screen_2.h"

namespace SingularityTrainer
{
QuadScreen2::QuadScreen2(ScreenManager *screen_manager, std::vector<std::shared_ptr<IScreen>> *screens, std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager)
{
    vertex_array = std::make_unique<VertexArray>();

    float vertices[] = {
        -0.5, 0.5, 1.0, 1.0, 1.0, 1.0,
        -0.5, -0.5, 0.0, 1.0, 0.0, 1.0,
        0.5, -0.5, 0.0, 0.0, 1.0, 1.0,
        0.5, 0.5, 1.0, 0.0, 1.0, 1.0};

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
    shader->bind();
}

QuadScreen2::~QuadScreen2() {}

void QuadScreen2::update(const float delta_time)
{
    ImGui::SetNextWindowSize(ImVec2(150, 50));
    ImGui::Begin("Screens", NULL, ImGuiWindowFlags_NoResize);
    int own_index = std::find(screen_names->begin(), screen_names->end(), "Quad test 2") - screen_names->begin();
    int screen_index = own_index;
    ImGui::Combo("", &screen_index, *screen_names);
    if (screen_index != own_index)
    {
        screen_manager->close_screen();
        screen_manager->show_screen((*screens)[screen_index]);
    }
    ImGui::End();
}

void QuadScreen2::draw(const float delta_time, const Renderer &renderer, bool lightweight)
{
    renderer.draw(*vertex_array, *element_buffer, *shader);
}
}