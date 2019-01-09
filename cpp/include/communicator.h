#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <msgpack.hpp>
#include <zmq.hpp>

#include "requests.h"

namespace SingularityTrainer
{
class Communicator
{
  public:
    Communicator(const std::string &url);
    ~Communicator();

    template <class T>
    void send_request(const Request<T> &request)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, request);

        zmq::message_t message(buffer.size());
        std::memcpy(message.data(), buffer.data(), buffer.size());
        socket->send(message);
    };

  private:
    std::unique_ptr<zmq::context_t> context;
    std::unique_ptr<zmq::socket_t> socket;
};
}