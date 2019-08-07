#pragma once

#include <memory>

#include <Box2D/Box2D.h>
#include <glm/mat4x4.hpp>

#include "misc/random.h"
#include "networking/client_agent.h"
#include "networking/client_communicator.h"
#include "screens/iscreen.h"
#include "third_party/di.hpp"
#include "training/bodies/body.h"
#include "training/environments/ienvironment.h"
#include "training/environments/playback_env.h"

namespace SingularityTrainer
{
class IO;
class PostProcLayer;
class Renderer;
class ResourceManager;

class MultiplayerScreen : public IScreen
{
  private:
    std::unique_ptr<ClientAgent> client_agent;
    std::unique_ptr<ClientCommunicator> client_communicator;
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    std::unique_ptr<PlaybackEnv> env;
    IO &io;
    glm::mat4 projection;
    ResourceManager &resource_manager;

  public:
    MultiplayerScreen(std::unique_ptr<ClientAgent> client_agent,
                      std::unique_ptr<ClientCommunicator> client_communicator,
                      std::unique_ptr<PlaybackEnv> env,
                      IO &io,
                      ResourceManager &resource_manager);

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class MultiplayerScreenFactory
{
  private:
    IEnvironmentFactory &env_factory;
    IO &io;
    ResourceManager &resource_manager;
    double tick_length;

  public:
    BOOST_DI_INJECT(MultiplayerScreenFactory,
                    IEnvironmentFactory &env_factory,
                    IO &io,
                    ResourceManager &resource_manager,
                    (named = TickLength) double tick_length)
        : env_factory(env_factory),
          io(io),
          resource_manager(resource_manager),
          tick_length(tick_length) {}

    virtual std::shared_ptr<IScreen> make(std::unique_ptr<ClientAgent> agent,
                                          std::unique_ptr<ClientCommunicator> client_communicator,
                                          int /*player_number*/)
    {
        return std::make_unique<MultiplayerScreen>(std::move(agent),
                                                   std::move(client_communicator),
                                                   std::make_unique<PlaybackEnv>(env_factory.make(), tick_length),
                                                   io,
                                                   resource_manager);
    }
};
}