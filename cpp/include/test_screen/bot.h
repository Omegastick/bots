#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "idrawable.h"
#include "resource_manager.h"
#include "test_screen/rigid_body.h"

namespace SingularityTrainer
{
class Bot : public IDrawable, public RigidBody
{
  public:
    Bot(const std::shared_ptr<ResourceManager> resource_manager, b2World &world);
    ~Bot();

    std::vector<std::string> labels;

    void act(std::vector<bool> actions);
    void draw(sf::RenderTarget &render_target);
    std::vector<float> get_observation();
    virtual void begin_contact(RigidBody &other);

  private:
    sf::Sprite sprite;
};
}