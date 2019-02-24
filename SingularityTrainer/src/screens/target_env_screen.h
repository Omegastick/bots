#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/render_data.h"
#include "communicator.h"
#include "iscreen.h"
#include "random.h"
#include "resource_manager.h"
#include "training/environments/target_env.h"
#include "training/trainers/itrainer.h"

namespace SingularityTrainer
{
class TargetEnvScreen : public IScreen
{
  private:
    std::unique_ptr<ITrainer> trainer;
    bool lightweight_rendering;
    glm::mat4 projection;
    bool fast;

  public:
    TargetEnvScreen(ResourceManager &resource_manager, Communicator *communicator, Random *rng, int env_count);
    ~TargetEnvScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(const float delta_time);
};
}