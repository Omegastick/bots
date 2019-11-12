#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class Communicator;
class PostProcLayer;
class ResourceManager;
class Random;
class Renderer;
class Trainer;

class KothEnvScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    std::unique_ptr<Trainer> trainer;
    bool lightweight_rendering;
    glm::mat4 projection;
    bool fast;

  public:
    KothEnvScreen(ResourceManager &resource_manager, Random &rng, int env_count);
    ~KothEnvScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};
}