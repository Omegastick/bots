#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"

namespace SingularityTrainer
{
class Body;
class ThrusterModule : public IModule, public IActivatable
{
  private:
    bool active;

  public:
    ThrusterModule();

    virtual void activate() override;
    virtual void sub_update() override;
    virtual nlohmann::json to_json() const override final;
    virtual void update() override;

    inline int get_observation_count() const override final { return 0; }
};
}