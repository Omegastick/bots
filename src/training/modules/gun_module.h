#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"
#include "training/entities/bullet.h"

namespace SingularityTrainer
{
class Body;
class Random;

class GunModule : public IModule, public IActivatable
{
  private:
    int cooldown;
    Random &rng;
    int steps_since_last_shot;

  public:
    GunModule(Random &rng);

    virtual RenderData get_render_data(bool lightweight = false);
    virtual void activate();
    virtual void update();
    virtual nlohmann::json to_json() const;

    inline int get_observation_count() const { return 0; }
};
}