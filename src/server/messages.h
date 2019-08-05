#pragma once

#include <vector>
#include <string>
#include <tuple>

#include <msgpack.hpp>

namespace SingularityTrainer
{
typedef std::tuple<float, float, float> Transform;

enum class MessageType
{
    Connect = 0,
    Action = 1,
    State = 2
};

struct Message
{
    MessageType type;
    MSGPACK_DEFINE_ARRAY(type)
};

struct ConnectMessage : Message
{
    std::string body_spec;

    ConnectMessage()
    {
        type = MessageType::Connect;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message), body_spec)
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

struct StateMessage : Message
{
    std::vector<Transform> agent_transforms;
    std::unordered_map<unsigned int, Transform> entity_states;
    int tick;

    StateMessage()
    {
        type = MessageType::State;
    }

    StateMessage(std::vector<Transform> agent_transforms,
                 std::unordered_map<unsigned int, Transform> entity_states,
                 int tick) : StateMessage()
    {
        this->agent_transforms = agent_transforms;
        this->entity_states = entity_states;
        this->tick = tick;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message),
                         agent_transforms,
                         entity_states,
                         tick)
};

MessageType get_message_type(msgpack::object &object)
{
    return object.via.array.ptr[0].as<MessageType>();
}
}

MSGPACK_ADD_ENUM(SingularityTrainer::MessageType)
