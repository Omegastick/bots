#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <zmq.hpp>
#include <msgpack.hpp>

#include "requests.h"

namespace SingularityTrainer
{
class Communicator
{
  public:
    Communicator(const std::string &url);
    ~Communicator();

    std::string get_raw_response();

    template <class T>
    std::shared_ptr<Response<T>> get_response()
    {
        // Receive message
        zmq::message_t packed_msg;
        socket->recv(&packed_msg);

        // Desrialize message
        msgpack::object_handle object_handle = msgpack::unpack(static_cast<char *>(packed_msg.data()), packed_msg.size());
        msgpack::object object = object_handle.get();

        // Fill out response object
        std::shared_ptr<Response<T>> response = std::make_shared<Response<T>>();
        object.convert(response);

        return response;
    }

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