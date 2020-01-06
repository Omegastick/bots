#pragma once

#include <vector>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "graphics/render_data.h"
#include "training/modules/imodule.h"

namespace ai
{
class Body;

class BaseModule : public IModule
{
  private:
    Rectangle rectangle;
    Circle circle;

  public:
    BaseModule();

    virtual void draw(Renderer &renderer, bool lightweight = false) override;
    virtual std::vector<float> get_sensor_reading() const override;
    virtual void set_color(const ColorScheme &color_scheme) override;
    virtual nlohmann::json to_json() const override;

    inline virtual int get_observation_count() const override final { return 3; }
};
}