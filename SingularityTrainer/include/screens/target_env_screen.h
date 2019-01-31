#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <Thor/Input.hpp>
#include <memory>

#include "communicator.h"
#include "iscreen.h"
#include "resource_manager.h"
#include "training/environments/target_env.h"
#include "training/trainers/itrainer.h"
#include "random.h"

namespace SingularityTrainer
{
class TargetEnvScreen : public IScreen
{
  public:
    TargetEnvScreen(ResourceManager &resource_manager, Communicator *communicator, Random *rng, int env_count);
    ~TargetEnvScreen();

    void draw(sf::RenderTarget &render_target);
    void update(const sf::Time &delta_time, const sf::Vector2f &mouse_position, const thor::ActionMap<Inputs> &action_map);

  private:
    std::unique_ptr<ITrainer> trainer;
    std::shared_ptr<sf::Shader> shader;
    sf::RenderTexture texture;
};
}