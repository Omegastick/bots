#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "graphics/render_data.h"
#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"
#include "training/entities/bullet.h"

namespace ai
{
class Body;
class IAudioEngine;
class IBulletFactory;
class Random;

class GunModule : public IModule, public IActivatable
{
  private:
    IAudioEngine &audio_engine;
    Rectangle barrel_rectangle, body_rectangle;
    IBulletFactory &bullet_factory;
    int cooldown;
    Random &rng;
    int steps_since_last_shot;

  public:
    GunModule(IAudioEngine &audio_engine, IBulletFactory &bullet_factory, Random &rng);

    virtual void activate() override;
    virtual void draw(Renderer &renderer, bool lightweight = false) override;
    virtual void update() override;
    virtual void set_color(const ColorScheme &color_scheme) override;
    virtual nlohmann::json to_json() const override final;

    inline virtual int get_observation_count() const override final { return 0; }
};
}