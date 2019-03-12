#pragma once

#include <vector>
#include <memory>

#include <glm/mat4x4.hpp>

#include "iscreen.h"

namespace SingularityTrainer
{
class ResourceManager;
class Random;
class Communicator;
class Renderer;
class PostProcLayer;
class IEnvironment;

class WatchScreen : public IScreen
{
  private:
    void action_update();

    enum States
    {
        BROWSING = 0,
        WATCHING = 1
    };
    ResourceManager *resource_manager;
    Communicator *communicator;
    glm::mat4 projection;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    std::unique_ptr<IEnvironment> environment;
    States state;
    int selected_file;
    int frame_counter;
    std::vector<std::vector<float>> observations;

  public:
    WatchScreen(ResourceManager &resource_manager, Communicator &communicator, Random &rng);
    ~WatchScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(const float delta_time);
};
}