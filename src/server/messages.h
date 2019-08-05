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
    ConnectConfirmation = 1,
    Action = 2,
    State = 3
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

struct ConnectConfirmationMessage : Message
{
    int player_number;

    ConnectConfirmationMessage(int player_number)
    {
        type = MessageType::ConnectConfirmation;
        this->player_number = player_number;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message), player_number)
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
    std::unordered_map<unsigned int, Transform> entity_transforms;
    int tick;

    StateMessage()
    {
        type = MessageType::State;
    }

    StateMessage(std::vector<Transform> agent_transforms,
                 std::unordered_map<unsigned int, Transform> entity_transforms,
                 int tick) : StateMessage()
    {
        this->agent_transforms = agent_transforms;
        this->entity_transforms = entity_transforms;
        this->tick = tick;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message),
                         agent_transforms,
                         entity_transforms,
                         tick)
};

inline MessageType get_message_type(const msgpack::object &object)
{
    return object.via.array.ptr[0].as<MessageType>();
}
}

MSGPACK_ADD_ENUM(SingularityTrainer::MessageType)
