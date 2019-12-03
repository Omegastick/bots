#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "screens/koth_env_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "graphics/colors.h"
#include "training/environments/koth_env.h"
#include "training/trainers/quick_trainer.h"
#include "screens/iscreen.h"
#include "misc/resource_manager.h"

namespace SingularityTrainer
{
KothEnvScreen::KothEnvScreen(ResourceManager &resource_manager, Random &rng, int env_count)
    : resource_manager(&resource_manager),
      lightweight_rendering(false),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      fast(false)
{
    trainer = std::make_unique<QuickTrainer>(&rng, env_count);

    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);
    resource_manager.load_font("roboto-32", "fonts/Roboto-Regular.ttf", 32);

    trainer->begin_training();
}

KothEnvScreen::~KothEnvScreen()
{
    trainer->end_training();
}

void KothEnvScreen::update(const double /*delta_time*/)
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
}

void KothEnvScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
    trainer->environments[0]->draw(renderer, lightweight_rendering);
}
}