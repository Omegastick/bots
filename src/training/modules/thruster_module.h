#pragma once

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>

#include "training/modules/imodule.h"
#include "training/modules/interfaces/iactivatable.h"

namespace SingularityTrainer
{
class Body;
struct RenderData;

class ThrusterModule : public IModule, public IActivatable
{
  private:
    bool active;

  public:
    ThrusterModule();

    virtual void activate();
    virtual void sub_update();
    virtual nlohmann::json to_json() const;
    virtual void update();

    inline int get_observation_count() const { return 0; }
};
}