#pragma once

#include <memory>
#include <string>

#include <msgpack.hpp>

#include "third_party/zmq.hpp"
#include "third_party/zmq_addon.hpp"

namespace SingularityTrainer
{
enum class MessageType
{
    Connect = 0,
    Action = 1,
    AsdAsd = 2
};

struct MessageWithId
{
    std::string id;
    std::string message;
};

class ServerCommunicator
{
  private:
    std::unique_ptr<zmq::socket_t> socket;

  public:
    ServerCommunicator(std::unique_ptr<zmq::socket_t> socket);

    MessageWithId get();
    void send(const std::string &client_id, const std::string &message);
};
}

MSGPACK_ADD_ENUM(SingularityTrainer::MessageType)
