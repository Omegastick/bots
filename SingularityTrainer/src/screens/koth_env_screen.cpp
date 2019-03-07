#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "screens/koth_env_screen.h"
#include "communicator.h"
#include "graphics/colors.h"
#include "iscreen.h"
#include "resource_manager.h"
#include "training/environments/koth_env.h"
#include "training/trainers/quick_trainer.h"

namespace SingularityTrainer
{
KothEnvScreen::KothEnvScreen(ResourceManager &resource_manager, Communicator *communicator, Random *rng, int env_count)
    : lightweight_rendering(false), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)), fast(false), resource_manager(&resource_manager)
{
    trainer = std::make_unique<QuickTrainer>(communicator, rng, env_count);

    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");

    crt_post_proc_layer = std::make_unique<PostProcLayer>(resource_manager.shader_store.get("crt").get());

    trainer->begin_training();
}

KothEnvScreen::~KothEnvScreen()
{
    trainer->end_training();
}

void KothEnvScreen::update(const float delta_time)
{
    ImGui::Begin("Speed");
    ImGui::Checkbox("Fast?", &fast);
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
}

void KothEnvScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.push_post_proc_layer(crt_post_proc_layer.get());
    renderer.begin();

    renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
    auto render_data = trainer->environments[0]->get_render_data(lightweight_rendering);
    renderer.draw(render_data, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f), trainer->environments[0]->get_elapsed_time(), lightweight_rendering);

    auto crt_shader = resource_manager->shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", glm::vec2(renderer.get_width(), renderer.get_height()));
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 0.8);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.03);

    renderer.end();
}
}