#pragma once

#include "graphics/render_data.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class SquareHull : public IModule
{
  private:
    Rectangle rectangle;

  public:
    SquareHull();

    virtual void draw(Renderer &renderer, bool lightweight = false) override;
    virtual void set_color(glm::vec4 color) override;
    virtual nlohmann::json to_json() const override final;

    inline virtual int get_observation_count() const override final { return 0; }
};
}