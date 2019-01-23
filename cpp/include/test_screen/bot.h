#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include <memory>
#include <string>
#include <vector>

#include "idrawable.h"
#include "resource_manager.h"
#include "test_screen/icollidable.h"
#include "test_screen/rigid_body.h"

namespace SingularityTrainer
{
class Bot : public IDrawable, public ICollidable
{
  public:
    Bot(ResourceManager &resource_manager, b2World &world);
    ~Bot();

    void act(std::vector<int> &actions);
    void draw(sf::RenderTarget &render_target);
    std::vector<float> get_observation();
    virtual void begin_contact(RigidBody *other);
    virtual void end_contact(RigidBody *other);

    std::unique_ptr<RigidBody> rigid_body;

  private:
    sf::Sprite sprite;
};
}