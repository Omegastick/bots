#pragma once

#include <vector>
#include <memory>

#include <cpprl/model/policy.h>
#include <cpprl/storage.h>
#include <glm/mat4x4.hpp>

#include "iscreen.h"

namespace cpprl
{
class NNBase;
}

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
    void show_checkpoint_selector();
    void show_agent_scores();

    enum States
    {
        BROWSING = 0,
        WATCHING = 1
    };
    cpprl::Policy policy;
    std::shared_ptr<cpprl::NNBase> nn_base;
    glm::mat4 projection;
    ResourceManager *resource_manager;
    Communicator *communicator;
    States state;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    std::unique_ptr<IEnvironment> environment;
    int selected_file;
    int frame_counter;
    torch::Tensor observations, hidden_states, masks;
    std::vector<std::vector<float>> scores;

  public:
    WatchScreen(ResourceManager &resource_manager, Random &rng);
    ~WatchScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(const double delta_time);
};
}