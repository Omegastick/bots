#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "screens/iscreen.h"

namespace ai
{
class PostProcLayer;
class ResourceManager;
class Random;
class Renderer;
class Trainer;

class TargetEnvScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    std::unique_ptr<Trainer> trainer;
    bool lightweight_rendering;
    glm::mat4 projection;
    bool fast;

  public:
    TargetEnvScreen(ResourceManager &resource_manager, Random &rng, int env_count);
    ~TargetEnvScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};
}