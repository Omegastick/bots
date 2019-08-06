#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

#include <doctest.h>

#include <msgpack.hpp>

#include "client_communicator.h"
#include "server_communicator.h"
#include "networking/messages.h"
#include "networking/msgpack_codec.h"
#include "third_party/zmq.hpp"

namespace SingularityTrainer
{
ClientCommunicator::ClientCommunicator(std::unique_ptr<zmq::socket_t> socket)
    : socket(std::move(socket)) {}

std::string ClientCommunicator::get()
{
    zmq::message_t message;
    socket->recv(message, zmq::recv_flags::dontwait);

    if (message.empty())
    {
        // No available message
        return "";
    }

    return std::string(static_cast<char *>(message.data()), message.size());
}

void ClientCommunicator::send(const std::string &message)
{
    socket->send(zmq::message_t(message.data(), message.size()), zmq::send_flags::none);
}

TEST_CASE("ClientCommunicator")
{
    zmq::context_t context;

    zmq::socket_t server_socket(context, zmq::socket_type::router);
    server_socket.bind("tcp://*:5566");

    auto client_socket = std::make_unique<zmq::socket_t>(context, zmq::socket_type::dealer);
    client_socket->setsockopt(ZMQ_IDENTITY, "Identity");
    client_socket->connect("tcp://localhost:5566");
    ClientCommunicator client(std::move(client_socket));

    SUBCASE("Messages are sent correctly")
    {
        ConnectMessage message_to_send("asd");
        client.send(MsgPackCodec::encode(message_to_send));

        zmq::multipart_t message;
        message.recv(server_socket, static_cast<int>(zmq::recv_flags::none));

        std::string identity(static_cast<char *>(message[0].data()), message[0].size() - 1);
        DOCTEST_CHECK(identity == "Identity");

        std::string received_message_raw(static_cast<char *>(message[1].data()), message[1].size());
        auto received_message = MsgPackCodec::decode<ConnectMessage>(received_message_raw);

        DOCTEST_CHECK(received_message.body_spec == message_to_send.body_spec);
    }

    SUBCASE("Messages are received correctly")
    {
        ConnectMessage connect_message("asd");
        client.send(MsgPackCodec::encode(connect_message));

        zmq::multipart_t server_received_message;
        server_received_message.recv(server_socket, static_cast<int>(zmq::recv_flags::none));
        std::string identity(static_cast<char *>(server_received_message[0].data()), server_received_message[0].size());
        server_socket.send(zmq::message_t(identity.data(), identity.size()), zmq::send_flags::sndmore);
        server_socket.send(zmq::message_t("asd"), zmq::send_flags::none);

        std::string received_message;
        while (received_message.empty())
        {
            received_message = client.get();
        }

        DOCTEST_CHECK(received_message.substr(0, 3) == "asd");
    }
}
}