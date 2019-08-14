#pragma once

#include <memory>
#include <string>
#include <vector>

#include <agones/sdk.h>
#include <argh.h>

#include "networking/game.h"
#include "networking/server_communicator.h"
#include "third_party/zmq.hpp"
#include "third_party/zmq_addon.hpp"

namespace SingularityTrainer
{
class ServerApp
{
  private:
    zmq::context_t zmq_context; // ZMQ context has to outlive the socket

    std::shared_ptr<agones::SDK> agones_sdk;
    std::unique_ptr<Game> game;
    std::vector<std::string> players;
    std::unique_ptr<ServerCommunicator> server_communicator;

    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    ServerApp(std::shared_ptr<agones::SDK> agones_sdk, std::unique_ptr<Game> game);

    int run(int argc, char *argv[]);
};
}
