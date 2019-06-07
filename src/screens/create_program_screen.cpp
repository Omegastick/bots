#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "create_program_screen.h"
#include "graphics/renderers/renderer.h"
#include "misc/io.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"

namespace SingularityTrainer
{

CreateProgramScreen::CreateProgramScreen(std::unique_ptr<TrainingProgram> program,
                                         IO &io,
                                         ResourceManager &resource_manager,
                                         ScreenManager &screen_manager)
    : io(io),
      program(std::move(program)),
      resource_manager(resource_manager),
      screen_manager(screen_manager),
      state(State::Body)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    crt_post_proc_layer = PostProcLayer(resource_manager.shader_store.get("crt").get(),
                                        io.get_resolution().x,
                                        io.get_resolution().y);
}

void CreateProgramScreen::algorithm()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Algorithm");
    ImGui::End();
}

void CreateProgramScreen::body()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Body");
    ImGui::End();
}

void CreateProgramScreen::checkpoint()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Checkpoint");
    ImGui::End();
}

void CreateProgramScreen::rewards()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Rewards");
    ImGui::End();
}

void CreateProgramScreen::save_load()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Save/load");
    ImGui::End();
}

void CreateProgramScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(&crt_post_proc_layer);
    renderer.begin();

    auto crt_shader = resource_manager.shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 0.5);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.1);

    renderer.end();
}

void CreateProgramScreen::update(double /*delta_time*/)
{
    switch (state)
    {
    case Algorithm:
        algorithm();
        break;
    case Body:
        body();
        break;
    case Checkpoint:
        checkpoint();
        break;
    case Rewards:
        rewards();
        break;
    case SaveLoad:
        save_load();
        break;
    }
}
}