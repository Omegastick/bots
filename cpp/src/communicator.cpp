#include <memory>
#include <string>
#include <zmq.hpp>

#include "communicator.h"
#include "requests.h"

namespace SingularityTrainer
{
Communicator::Communicator(const std::string &url)
{
    context = std::make_unique<zmq::context_t>(1);
    socket = std::make_unique<zmq::socket_t>(*context, ZMQ_PAIR);

    socket->connect(url);
    socket->send(zmq::message_t("Connecting...", 13));
    std::cout << get_raw_response() << std::endl;
}

Communicator::~Communicator() {}

std::string Communicator::get_raw_response()
{
    // Receive message
    zmq::message_t zmq_msg;
    socket->recv(&zmq_msg);

    // Cast message to string
    std::string response = std::string(static_cast<char *>(zmq_msg.data()), zmq_msg.size());

    return response;
}
}
