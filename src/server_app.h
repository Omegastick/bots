#pragma once

#include <memory>
#include <string>
#include <vector>

#include <argh.h>

#include "networking/game.h"
#include "networking/server_communicator.h"
#include "third_party/httplib.h"
#include "third_party/zmq.hpp"
#include "third_party/zmq_addon.hpp"

namespace ai
{
class ServerApp
{
  private:
    zmq::context_t zmq_context; // ZMQ context has to outlive the socket

    std::string cloud_token;
    std::unique_ptr<Game> game;
    httplib::Client http_client;
    std::vector<std::string> player_tokens;
    std::vector<std::string> player_usernames;
    std::vector<std::string> players;
    std::unique_ptr<ServerCommunicator> server_communicator;

    int run_tests(int argc, char *argv[], const argh::parser &args);
    void update_elos(int victor);
    void wait_for_player_info();

  public:
    ServerApp(std::unique_ptr<Game> game);

    int run(int argc, char *argv[]);
};
}
