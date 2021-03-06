#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "graphics/render_data.h"
#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"

namespace ai
{
class ThrusterModule : public IModule, public IActivatable
{
  private:
    bool active;
    Trapezoid trapezoid;

  public:
    ThrusterModule();

    virtual void activate() override;
    virtual void draw(Renderer &renderer, bool lightweight = false) override;
    virtual void set_color(const ColorScheme &color_scheme) override;
    virtual void sub_update() override;
    virtual nlohmann::json to_json() const override final;
    virtual void update() override;

    inline int get_observation_count() const override final { return 0; }
};
}