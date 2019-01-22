#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "training/actions/iaction.h"
#include "training/modules/imodule.h"
#include "training/modules/interfaces/ishootable.h"

namespace SingularityTrainer
{
class GunModule : public IModule, public IShootable
{
  public:
    GunModule();
    ~GunModule();

    virtual std::vector<IModule *> get_children();
    virtual void act(std::vector<int> actions);
    virtual std::vector<float> get_sensor_reading();
    virtual void draw(sf::RenderTarget &render_target);
    virtual void shoot();

  private:
    int cooldown;
    int steps_since_last_shot;
};
}