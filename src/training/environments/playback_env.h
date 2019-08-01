#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include "graphics/render_data.h"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
struct EntityState
{
    unsigned int id;
    b2Transform transform;
};

struct EnvState
{
    std::vector<b2Transform> agent_transforms;
    std::vector<EntityState> entity_states;
    int tick;
};

class PlaybackEnv
{
  private:
    double current_tick;
    std::unique_ptr<IEnvironment> env;
    std::vector<EnvState> states;
    double tick_length;

  public:
    PlaybackEnv(std::unique_ptr<IEnvironment> env, double tick_length);

    void add_new_state(EnvState state);
    RenderData get_render_data(bool lightweight = false);
    void update(double delta_time);

    inline IEnvironment &get_env() { return *env; }
};
}