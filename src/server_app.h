#pragma once

#include <memory>
#include <string>
#include <vector>

#include <argh.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "networking/game.h"
#include "networking/server_communicator.h"

namespace SingularityTrainer
{
class ServerApp
{
  private:
    std::unique_ptr<Game> game;
    std::vector<std::string> players;
    std::unique_ptr<ServerCommunicator> server_communicator;
    zmq::context_t zmq_context;

    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    ServerApp(std::unique_ptr<Game> game);

    int run(int argc, char *argv[]);
};
}
