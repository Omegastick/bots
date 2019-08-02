#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

#include <doctest.h>

#include <msgpack.hpp>

#include "server_communicator.h"
#include "third_party/zmq.hpp"
#include "third_party/zmq_addon.hpp"

namespace SingularityTrainer
{
ServerCommunicator::ServerCommunicator(std::unique_ptr<zmq::socket_t> socket)
    : socket(std::move(socket)) {}

MessageWithId ServerCommunicator::get()
{
    zmq::multipart_t message;
    message.recv(*socket, static_cast<int>(zmq::recv_flags::dontwait));

    if (message.empty())
    {
        // No available message
        return {};
    }

    return {std::string(static_cast<char *>(message[0].data()), message[0].size() - 1),
            std::string(static_cast<char *>(message[1].data()), message[1].size())};
}

void ServerCommunicator::send(const std::string &client_id, const std::string &message) {}

TEST_CASE("ServerCommunicator")
{
    zmq::context_t context;

    auto server_socket = std::make_unique<zmq::socket_t>(context, zmq::socket_type::router);
    server_socket->bind("tcp://*:5566");
    ServerCommunicator server(std::move(server_socket));

    zmq::socket_t client_socket(context, zmq::socket_type::dealer);
    client_socket.setsockopt(ZMQ_IDENTITY, "Identity");
    client_socket.connect("tcp://localhost:5566");

    SUBCASE("Identity is received correctly")
    {
        std::string message_to_send("Hello");
        client_socket.send(zmq::message_t(message_to_send.data(), message_to_send.size()),
                           zmq::send_flags::none);

        MessageWithId received;
        while (received.id.empty())
        {
            received = server.get();
        }

        DOCTEST_CHECK(received.id == "Identity");
    }

    SUBCASE("Messages are received correctly")
    {
        std::stringstream buffer;
        typedef std::tuple<MessageType, std::vector<int>> ActionMessage;
        ActionMessage message_to_send{MessageType::Action, {1, 0, 1, 1}};
        msgpack::pack(buffer, message_to_send);
        auto message_str = buffer.str();
        client_socket.send(zmq::message_t(message_str.data(), message_str.size()),
                           zmq::send_flags::none);

        MessageWithId received_message_raw;
        while (received_message_raw.id.empty())
        {
            received_message_raw = server.get();
        }

        auto received_message = msgpack::unpack(received_message_raw.message.data(),
                                                received_message_raw.message.size())
                                    ->as<ActionMessage>();
        DOCTEST_CHECK(std::get<0>(received_message) == MessageType::Action);
        DOCTEST_CHECK(std::get<1>(received_message) == std::vector<int>{1, 0, 1, 1});
    }

    SUBCASE("Messages are sent correctly")
    {
        std::string handshake_message("Hello");
        client_socket.send(zmq::message_t(handshake_message.data(), handshake_message.size()),
                           zmq::send_flags::none);
        auto received_handhake = server.get();

        std::stringstream buffer;
        typedef std::tuple<MessageType, std::vector<int>> ActionMessage;
        ActionMessage message_to_send{MessageType::Action, {1, 0, 1, 1}};
        msgpack::pack(buffer, message_to_send);
        auto message_str = buffer.str();

        server.send(received_handhake.id, message_str);

        zmq::message_t received_message_raw;
        client_socket.recv(received_message_raw, zmq::recv_flags::none);

        auto received_message = msgpack::unpack(static_cast<char *>(received_message_raw.data()),
                                                received_message_raw.size())
                                    ->as<ActionMessage>();
        DOCTEST_CHECK(std::get<0>(received_message) == MessageType::Action);
        DOCTEST_CHECK(std::get<1>(received_message) == std::vector<int>{1, 0, 1, 1});
    }
}
}