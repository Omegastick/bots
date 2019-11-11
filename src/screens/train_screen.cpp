#include <chrono>
#include <thread>
#include <memory>
#include <mutex>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <imgui.h>
#include <fmt/format.h>

#include "train_screen.h"
#include "graphics/post_processing/distortion_layer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "graphics/colors.h"
#include "misc/io.h"
#include "misc/resource_manager.h"
#include "screens/iscreen.h"
#include "training/environments/koth_env.h"
#include "ui/back_button.h"

namespace SingularityTrainer
{
TrainScreen::TrainScreen(std::unique_ptr<TrainInfoWindow> train_info_window,
                         std::unique_ptr<Trainer> trainer,
                         IO &io,
                         ResourceManager &resource_manager,
                         ScreenManager &screen_manager)
    : batch_finished(true),
      crt_post_proc_layer(
          std::make_unique<PostProcLayer>(*resource_manager.shader_store.get("crt"))),
      fast(false),
      io(io),
      last_eval_time(std::chrono::high_resolution_clock::now()),
      lightweight_rendering(false),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      resource_manager(resource_manager),
      screen_manager(screen_manager),
      train_info_window(std::move(train_info_window)),
      trainer(std::move(trainer))
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("square_hull", "images/square_hull.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_shader("distortion",
                                 "shaders/distortion.vert",
                                 "shaders/distortion.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    auto resolution = io.get_resolution();
    distortion_layer = std::make_unique<DistortionLayer>(
        *resource_manager.shader_store.get("distortion"), resolution.x, resolution.y);
}

TrainScreen::~TrainScreen()
{
    trainer->set_fast();
    if (batch_thread.joinable())
    {
        batch_thread.join();
    }
}

void TrainScreen::update(const double /*delta_time*/)
{
    glm::vec2 resolution = io.get_resolutionf();
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.05f}, ImGuiCond_Once);
    ImGui::Begin("Speed", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    ImGui::Checkbox("Fast", &fast);
    ImGui::End();
    if (fast)
    {
        lightweight_rendering = true;
        trainer->set_fast();
    }
    else
    {
        lightweight_rendering = false;
        trainer->set_slow();
    }
    if (batch_finished)
    {
        if (batch_thread.joinable())
        {
            batch_thread.join();
        }
        batch_finished = false;
        batch_thread = std::thread([&] {
            const auto batch_data = trainer->step_batch();
            const auto timestep = trainer->get_timestep();
            {
                std::lock_guard lock_guard(train_info_window_mutex);
                for (const auto &datum : batch_data)
                {
                    train_info_window->add_data(datum.first, timestep, datum.second);
                }
            }

            const auto now = std::chrono::high_resolution_clock::now();
            if (now - last_eval_time > std::chrono::seconds(60))
            {
                const auto timestep = trainer->get_timestep();
                const auto elo = trainer->evaluate();
                train_info_window->add_data("Elo", timestep, elo);
                last_eval_time = now;
            }

            batch_finished = true;
        });
    }

    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.15f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.3f}, ImGuiCond_Once);
    ImGui::Begin("Health");
    auto bodies = trainer->get_environments()[0]->get_bodies();
    for (const auto &body : bodies)
    {
        const float health = body->get_hp();
        const float max_health = 10;
        ImGui::ProgressBar(health / max_health, {-1, 0}, fmt::format("{}/{}", health, max_health).c_str());
    }
    ImGui::End();

    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::Begin("Scores");
    for (const auto &score : trainer->get_environments()[0]->get_scores())
    {
        ImGui::Text("%.1f", score);
    }
    ImGui::End();

    ImGui::SetNextWindowSize({resolution.x * 0.15f, resolution.y * 0.075f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.7f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::Begin("Current opponent");
    const auto opponent = trainer->get_current_opponent(0);
    ImGui::Text("Current opponent: %s", opponent.c_str());
    ImGui::End();

    {
        std::lock_guard lock_guard(train_info_window_mutex);
        train_info_window->update(trainer->get_timestep(), trainer->get_batch_number());
    }

    back_button(screen_manager, resolution);
}

void TrainScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_distortion_layer(*distortion_layer);
    renderer.push_post_proc_layer(*crt_post_proc_layer);

    if (trainer->should_clear_particles())
    {
        renderer.clear_particles();
    }

    const double view_height = 50;
    auto view_top = view_height * 0.5;
    glm::vec2 resolution = io.get_resolutionf();
    auto view_right = view_top * (resolution.x / resolution.y);
    projection = glm::ortho(-view_right, view_right, -view_top, view_top);

    renderer.scissor(-10, -20, 10, 20, projection);
    renderer.set_view(projection);
    trainer->draw(renderer, lightweight_rendering);

    auto crt_shader = resource_manager.shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
}
}