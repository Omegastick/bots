#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/fmt/fmt.h>

#include "train_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/post_proc_layer.h"
#include "graphics/colors.h"
#include "misc/io.h"
#include "misc/resource_manager.h"
#include "training/environments/koth_env.h"
#include "training/trainers/quick_trainer.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
TrainScreen::TrainScreen(std::unique_ptr<ITrainer> trainer,
                         IO &io,
                         ResourceManager &resource_manager)
    : crt_post_proc_layer(std::make_unique<PostProcLayer>(resource_manager.shader_store.get("crt").get())),
      fast(false),
      io(io),
      lightweight_rendering(false),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      resource_manager(resource_manager),
      trainer(std::move(trainer))
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);
}

void TrainScreen::update(const double /*delta_time*/)
{
    ImGui::Begin("Speed", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    ImGui::Checkbox("Fast", &fast);
    ImGui::End();
    if (fast)
    {
        lightweight_rendering = true;
        trainer->step();
    }
    else
    {
        lightweight_rendering = false;
        trainer->slow_step();
    }

    glm::vec2 resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.3f}, ImGuiCond_Once);
    ImGui::Begin("Health");
    auto bodies = trainer->environments[0]->get_bodies();
    for (const auto &body : bodies)
    {
        auto health = body->get_hp();
        double max_health = 10;
        ImGui::ProgressBar(health / max_health, {-1, 0}, fmt::format("{}/{}", health, max_health).c_str());
    }
    ImGui::End();

    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::Begin("Rewards");
    for (const auto &reward : trainer->environments[0]->get_total_rewards())
    {
        ImGui::Text("%.1f", reward);
    }
    ImGui::End();

    // ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.7f}, ImGuiCond_Once);
    ImGui::Begin("Observations");
    auto observations = trainer->get_observation();
    ImGui::PlotLines("##observations", observations.data(), observations.size(), 0, nullptr, 0, 1);
    ImGui::End();
}

void TrainScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(crt_post_proc_layer.get());
    renderer.begin();

    renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
    auto render_data = trainer->environments[0]->get_render_data(lightweight_rendering);
    renderer.draw(render_data, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f), trainer->environments[0]->get_elapsed_time(), lightweight_rendering);

    auto crt_shader = resource_manager.shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 0.8);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.1);

    renderer.end();
}
}