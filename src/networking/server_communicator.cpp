#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

#include <doctest.h>

#include <msgpack.hpp>

#include "server_communicator.h"
#include "networking/messages.h"
#include "third_party/zmq.hpp"
#include "third_party/zmq_addon.hpp"

namespace ai
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

    return {std::string(static_cast<char *>(message[0].data()), message[0].size()),
            std::string(static_cast<char *>(message[1].data()), message[1].size())};
}

void ServerCommunicator::send(const std::string &client_id, const std::string &message)
{
    socket->send(zmq::message_t(client_id.data(), client_id.size()),
                 zmq::send_flags::dontwait | zmq::send_flags::sndmore);
    socket->send(zmq::message_t(message.data(), message.size()),
                 zmq::send_flags::dontwait);
}

TEST_CASE("ServerCommunicator")
{
    zmq::context_t context;

    auto server_socket = std::make_unique<zmq::socket_t>(context, zmq::socket_type::router);
    server_socket->bind("tcp://*:5566");
    ServerCommunicator server(std::move(server_socket));

    zmq::socket_t client_socket(context, zmq::socket_type::dealer);
    std::string identity("Identity");
    client_socket.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.size());
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
        ActionMessage message_to_send({1, 0, 1, 1}, 1);
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
        DOCTEST_CHECK(received_message.type == message_to_send.type);
        DOCTEST_CHECK(received_message.actions == message_to_send.actions);
        DOCTEST_CHECK(received_message.tick == message_to_send.tick);
    }

    SUBCASE("Messages are sent correctly")
    {
        std::string handshake_message("Hello");
        client_socket.send(zmq::message_t(handshake_message.data(), handshake_message.size()),
                           zmq::send_flags::none);
        MessageWithId received_handshake;
        while (received_handshake.id.empty())
        {
            received_handshake = server.get();
        }

        std::stringstream buffer;
        ActionMessage message_to_send({1, 0, 1, 1}, 8);
        msgpack::pack(buffer, message_to_send);
        auto message_str = buffer.str();

        server.send(received_handshake.id, message_str);

        zmq::message_t received_message_raw;
        client_socket.recv(received_message_raw, zmq::recv_flags::none);

        auto received_message = msgpack::unpack(static_cast<char *>(received_message_raw.data()),
                                                received_message_raw.size())
                                    ->as<ActionMessage>();

        DOCTEST_CHECK(received_message.type == message_to_send.type);
        DOCTEST_CHECK(received_message.actions == message_to_send.actions);
        DOCTEST_CHECK(received_message.tick == message_to_send.tick);
    }
}
}