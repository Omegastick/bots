#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "iscreen.h"

namespace SingularityTrainer
{
class ITrainer;
class ResourceManager;
class Random;
class Renderer;
class PostProcLayer;

class TargetEnvScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    std::unique_ptr<ITrainer> trainer;
    bool lightweight_rendering;
    glm::mat4 projection;
    bool fast;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;

  public:
    TargetEnvScreen(ResourceManager &resource_manager, Random &rng, int env_count);
    ~TargetEnvScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(const float delta_time);
};
}