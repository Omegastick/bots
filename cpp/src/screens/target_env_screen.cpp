#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <Thor/Input.hpp>
#include <memory>

#include "communicator.h"
#include "gui/colors.h"
#include "iscreen.h"
#include "resource_manager.h"
#include "screens/target_env_screen.h"
#include "training/environments/target_env.h"
#include "training/trainers/quick_trainer.h"

namespace SingularityTrainer
{
TargetEnvScreen::TargetEnvScreen(ResourceManager &resource_manager, Communicator *communicator, int env_count)
{
    trainer = std::make_unique<QuickTrainer>(resource_manager, communicator, env_count);

    resource_manager.load_shader("crt", "shaders/crt.frag");
    shader = resource_manager.shader_store.get("crt");
    shader->setUniform("texture", sf::Shader::CurrentTexture);
    shader->setUniform("resolution", sf::Vector2f(1920, 1080));
    shader->setUniform("output_gamma", 1.1f);
    shader->setUniform("strength", 0.3f);
    shader->setUniform("distortion_factor", 0.08f);

    texture.create(1920, 1080);
}

TargetEnvScreen::~TargetEnvScreen() {}

void TargetEnvScreen::update(const sf::Time &delta_time, const sf::Vector2f &mouse_position, const thor::ActionMap<Inputs> &action_map)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        trainer->slow_step();
    }
    else
    {
        trainer->step();
    }
}

void TargetEnvScreen::draw(sf::RenderTarget &render_target)
{
    texture.clear(cl_background);

    trainer->environments[0]->draw(texture);

    texture.display();
    sf::Vector2u resolution = render_target.getSize();
    shader->setUniform("resolution", sf::Vector2f(resolution.x, resolution.y));
    render_target.draw(sf::Sprite(texture.getTexture()), shader.get());
}
}