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
    GameStart = 3,
    State = 4
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

    ConnectMessage(const std::string &body_spec) : ConnectMessage()
    {
        this->body_spec = body_spec;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message), body_spec)
};

struct ConnectConfirmationMessage : Message
{
    int player_number;

    ConnectConfirmationMessage()
    {
        type = MessageType::ConnectConfirmation;
    }

    ConnectConfirmationMessage(int player_number) : ConnectConfirmationMessage()
    {
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

struct GameStartMessage : Message
{
    std::vector<std::string> body_specs;

    GameStartMessage()
    {
        type = MessageType::GameStart;
    }

    GameStartMessage(std::vector<std::string> body_specs) : GameStartMessage()
    {
        this->body_specs = body_specs;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message), body_specs)
};

struct StateMessage : Message
{
    std::vector<Transform> agent_transforms;
    std::unordered_map<unsigned int, Transform> entity_transforms;
    int tick;
    bool done;

    StateMessage()
    {
        type = MessageType::State;
    }

    StateMessage(std::vector<Transform> agent_transforms,
                 std::unordered_map<unsigned int, Transform> entity_transforms,
                 bool done,
                 int tick) : StateMessage()
    {
        this->agent_transforms = agent_transforms;
        this->entity_transforms = entity_transforms;
        this->done = done;
        this->tick = tick;
    }

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Message),
                         agent_transforms,
                         entity_transforms,
                         tick,
                         done)
};

inline MessageType get_message_type(const msgpack::object &object)
{
    return static_cast<MessageType>(object.via.array.ptr[0].via.array.ptr[0].as<int>());
}
}

MSGPACK_ADD_ENUM(SingularityTrainer::MessageType)
