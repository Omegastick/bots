#pragma once

#include <vector>
#include <memory>

#include <cpprl/model/policy.h>
#include <cpprl/storage.h>
#include <glm/mat4x4.hpp>

#include "screens/iscreen.h"
#include "training/bodies/body.h"
#include "training/environments/ienvironment.h"
#include "training/training_program.h"
#include "ui/watch_screen/checkpoint_selector_window.h"

namespace cpprl
{
class NNBase;
}

namespace SingularityTrainer
{
class Communicator;
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

    enum class States
    {
        BROWSING = 0,
        WATCHING = 1
    };
    CheckpointSelectorWindow checkpoint_selector_window;
    std::unique_ptr<IEnvironment> environment;
    cpprl::Policy policy;
    std::shared_ptr<cpprl::NNBase> nn_base;
    glm::mat4 projection;
    ResourceManager *resource_manager;
    Communicator *communicator;
    States state;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    int frame_counter;
    std::vector<torch::Tensor> observations;
    torch::Tensor hidden_states, masks;
    std::vector<std::vector<float>> scores;

  public:
    WatchScreen(std::unique_ptr<IEnvironment> environment, ResourceManager &resource_manager, IO &io);
    ~WatchScreen();

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class WatchScreenFactory : public IScreenFactory
{
  private:
    BodyFactory &body_factory;
    IEnvironmentFactory &env_factory;
    IO &io;
    ResourceManager &resource_manager;
    Random &rng;

  public:
    WatchScreenFactory(BodyFactory &body_factory,
                       IEnvironmentFactory &env_factory,
                       IO &io,
                       ResourceManager &resource_manager,
                       Random &rng)
        : body_factory(body_factory),
          env_factory(env_factory),
          io(io),
          resource_manager(resource_manager),
          rng(rng) {}

    inline std::shared_ptr<IScreen> make()
    {
        auto world = std::make_unique<b2World>(b2Vec2_zero);
        std::vector<std::unique_ptr<Body>> bodies;
        bodies.push_back(body_factory.make(*world, rng));
        bodies.push_back(body_factory.make(*world, rng));
        return std::make_shared<WatchScreen>(env_factory.make(0, std::move(world), std::move(bodies), RewardConfig()),
                                             resource_manager, io);
    }
};
}