#pragma once

#include <memory>
#include <string>

#include "third_party/zmq.hpp"
#include "third_party/zmq_addon.hpp"

namespace ai
{
class ClientCommunicator
{
  private:
    std::unique_ptr<zmq::socket_t> socket;

  public:
    ClientCommunicator(std::unique_ptr<zmq::socket_t> socket);

    std::string get();
    void send(const std::string &message);
};
}