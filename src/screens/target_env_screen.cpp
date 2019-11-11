#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "screens/target_env_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/post_proc_layer.h"
#include "graphics/colors.h"
#include "training/trainers/quick_trainer.h"
#include "screens/iscreen.h"
#include "misc/resource_manager.h"

namespace SingularityTrainer
{
TargetEnvScreen::TargetEnvScreen(ResourceManager &resource_manager, Random &rng, int env_count)
    : resource_manager(&resource_manager),
      lightweight_rendering(false),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      fast(false)

{
    trainer = std::make_unique<QuickTrainer>(&rng, env_count);

    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    crt_post_proc_layer = std::make_unique<PostProcLayer>(*resource_manager.shader_store.get("crt"));

    trainer->begin_training();
}

TargetEnvScreen::~TargetEnvScreen()
{
    trainer->end_training();
}

void TargetEnvScreen::update(const double /*delta_time*/)
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

void TargetEnvScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(*crt_post_proc_layer);

    renderer.scissor(-10, -10, 10, 10, glm::ortho(-19.2f, 19.2f, -10.8f, 10.8f));
    trainer->environments[0]->draw(renderer, lightweight_rendering);

    auto crt_shader = resource_manager->shader_store.get("crt");
    crt_shader.set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
}
}