#pragma once

#include <vector>

#include <msgpack.hpp>

namespace SingularityTrainer
{
enum class MessageType
{
    Connect = 0,
    Action = 1
};

struct Message
{
    MessageType type;
    MSGPACK_DEFINE_ARRAY(type)
};

struct ConnectMessage : Message
{
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message))
};

struct ActionMessage : Message
{
    std::vector<int> actions;
    int tick;

    ActionMessage()
    {
        type = MessageType::Action;
    }

    ActionMessage(std::vector<int> actions, int tick) : ActionMessage()
    {
        this->actions = actions;
        this->tick = tick;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message), actions, tick)
};

MessageType get_message_type(msgpack::object &object)
{
    return object.via.array.ptr[0].as<MessageType>();
}
}

MSGPACK_ADD_ENUM(SingularityTrainer::MessageType)
