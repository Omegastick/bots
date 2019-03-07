#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/post_proc_layer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "communicator.h"
#include "iscreen.h"
#include "random.h"
#include "resource_manager.h"
#include "training/environments/target_env.h"
#include "training/trainers/itrainer.h"

namespace SingularityTrainer
{
class KothEnvScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    std::unique_ptr<ITrainer> trainer;
    bool lightweight_rendering;
    glm::mat4 projection;
    bool fast;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;

  public:
    KothEnvScreen(ResourceManager &resource_manager, Communicator *communicator, Random *rng, int env_count);
    ~KothEnvScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(const float delta_time);
};
}