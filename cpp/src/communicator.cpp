#include <memory>
#include <zmq.hpp>

#include "communicator.h"

namespace SingularityTrainer
{
Communicator::Communicator(const std::string &url)
{
    context = std::make_unique<zmq::context_t>(1);
    socket = std::make_unique<zmq::socket_t>(*context, ZMQ_PAIR);

    socket->connect(url);
    socket->send(zmq::message_t("Connecting...", 13));
};

Communicator::~Communicator(){};
}
