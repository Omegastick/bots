#pragma once

#include <memory>

#include <Box2D/Box2D.h>
#include <glm/mat4x4.hpp>

#include "misc/random.h"
#include "networking/client_agent.h"
#include "networking/client_communicator.h"
#include "screens/iscreen.h"
#include "third_party/di.hpp"
#include "training/agents/iagent.h"
#include "training/checkpointer.h"
#include "training/bodies/body.h"
#include "training/environments/playback_env.h"
#include "ui/multiplayer_screen/choose_agent_window.h"

namespace SingularityTrainer
{
class BodyFactory;
class IO;
class PostProcLayer;
class Random;
class Renderer;
class ResourceManager;

class MultiplayerScreen : public IScreen
{
  private:
    enum class State
    {
        ChooseAgent = 0,
        InputAddress = 1,
        WaitingToStart = 2,
        Playing = 3
    };

    zmq::context_t zmq_context;

    std::unique_ptr<IAgent> agent;
    std::unique_ptr<ChooseAgentWindow> choose_agent_window;
    std::unique_ptr<ClientAgent> client_agent;
    std::unique_ptr<ClientCommunicator> client_communicator;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    std::unique_ptr<PlaybackEnv> env;
    IEnvironmentFactory &env_factory;
    IO &io;
    glm::mat4 projection;
    ResourceManager &resource_manager;
    Random &rng;
    std::string server_address;
    State state;
    double tick_length;

    void choose_agent();
    void input_address();
    void play(double delta_time);
    void wait_for_start();

  public:
    MultiplayerScreen(double tick_length,
                      std::unique_ptr<ChooseAgentWindow> choose_agent_window,
                      IEnvironmentFactory &env_factory,
                      IO &io,
                      ResourceManager &resource_manager,
                      Random &rng);

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class MultiplayerScreenFactory
{
  private:
    Checkpointer &checkpointer;
    IEnvironmentFactory &env_factory;
    IO &io;
    ResourceManager &resource_manager;
    Random &rng;
    double tick_length;

  public:
    BOOST_DI_INJECT(MultiplayerScreenFactory,
                    Checkpointer &checkpointer,
                    IEnvironmentFactory &env_factory,
                    IO &io,
                    ResourceManager &resource_manager,
                    Random &rng,
                    (named = TickLength) double tick_length)
        : checkpointer(checkpointer),
          env_factory(env_factory),
          io(io),
          resource_manager(resource_manager),
          rng(rng),
          tick_length(tick_length) {}

    virtual std::shared_ptr<IScreen> make()
    {
        return std::make_unique<MultiplayerScreen>(tick_length,
                                                   std::make_unique<ChooseAgentWindow>(checkpointer, io),
                                                   env_factory,
                                                   io,
                                                   resource_manager,
                                                   rng);
    }
};
}