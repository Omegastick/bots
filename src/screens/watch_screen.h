#pragma once

#include <vector>
#include <memory>

#include <cpprl/model/policy.h>
#include <cpprl/storage.h>
#include <glm/mat4x4.hpp>

#include "screens/iscreen.h"
#include "ui/watch_screen/checkpoint_selector_window.h"

namespace cpprl
{
class NNBase;
}

namespace SingularityTrainer
{
class Communicator;
class IEnvironment;
class IO;
class PostProcLayer;
class Random;
class Renderer;
class ResourceManager;

class WatchScreen : public IScreen
{
  private:
    void action_update();
    void show_checkpoint_selector();
    void show_agent_scores();

    enum States
    {
        BROWSING = 0,
        WATCHING = 1
    };
    CheckpointSelectorWindow checkpoint_selector_window;
    cpprl::Policy policy;
    std::shared_ptr<cpprl::NNBase> nn_base;
    glm::mat4 projection;
    ResourceManager *resource_manager;
    Communicator *communicator;
    States state;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    std::unique_ptr<IEnvironment> environment;
    int frame_counter;
    torch::Tensor observations, hidden_states, masks;
    std::vector<std::vector<float>> scores;

  public:
    WatchScreen(ResourceManager &resource_manager, IO &io);
    ~WatchScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(const double delta_time);
};
}