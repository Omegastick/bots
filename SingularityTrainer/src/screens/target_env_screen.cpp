#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "communicator.h"
#include "graphics/colors.h"
#include "iscreen.h"
#include "resource_manager.h"
#include "screens/target_env_screen.h"
#include "training/environments/target_env.h"
#include "training/trainers/quick_trainer.h"

namespace SingularityTrainer
{
TargetEnvScreen::TargetEnvScreen(ResourceManager &resource_manager, Communicator *communicator, Random *rng, int env_count)
    : lightweight_rendering(false), projection(glm::ortho(0.f, 19.2f, 0.f, 10.8f))
{
    trainer = std::make_unique<QuickTrainer>(resource_manager, communicator, rng, env_count);

    // resource_manager.load_shader("crt", "shaders/crt.frag");
    // shader = resource_manager.shader_store.get("crt");
    // shader->setUniform("texture", sf::Shader::CurrentTexture);
    // shader->setUniform("resolution", sf::Vector2f(1920, 1080));
    // shader->setUniform("output_gamma", 1.1f);
    // shader->setUniform("strength", 0.3f);
    // shader->setUniform("distortion_factor", 0.08f);

    trainer->begin_training();
}

TargetEnvScreen::~TargetEnvScreen()
{
    trainer->end_training();
}

void TargetEnvScreen::update(const float delta_time)
{
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        lightweight_rendering = false;
        trainer->slow_step();
    }
    // else
    // {
    //     lightweight_rendering = true;
    //     trainer->step();
    // }
}

void TargetEnvScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    auto render_data = trainer->environments[0]->get_render_data(lightweight_rendering);
    renderer.draw(render_data);

    renderer.end();
}
}