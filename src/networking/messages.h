#pragma once

#include <vector>
#include <string>
#include <tuple>

#include <msgpack.hpp>

#include "misc/random.h"
#include "training/events/ievent.h"
#include "training/events/effect_triggered.h"
#include "training/events/entity_destroyed.h"

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
    std::vector<std::unique_ptr<IEvent>> events;
    std::vector<float> hps;
    std::vector<float> scores;
    int tick;
    bool done;

    StateMessage()
    {
        type = MessageType::State;
    }

    StateMessage(std::vector<Transform> agent_transforms,
                 std::unordered_map<unsigned int, Transform> entity_transforms,
                 std::vector<std::unique_ptr<IEvent>> events,
                 std::vector<float> hps,
                 std::vector<float> scores,
                 bool done,
                 int tick) : StateMessage()
    {
        this->agent_transforms = std::move(agent_transforms);
        this->entity_transforms = std::move(entity_transforms);
        this->events = std::move(events);
        this->hps = std::move(hps);
        this->scores = std::move(scores);
        this->done = done;
        this->tick = tick;
    }
};

inline MessageType get_message_type(const msgpack::object &object)
{
    return static_cast<MessageType>(object.via.array.ptr[0].via.array.ptr[0].as<int>());
}
}

MSGPACK_ADD_ENUM(SingularityTrainer::MessageType)

namespace msgpack
{
using namespace SingularityTrainer;

MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
{
    namespace adaptor
    {
    template <>
    struct pack<IEvent>
    {
        template <typename Stream>
        packer<Stream> &operator()(msgpack::packer<Stream> &o, IEvent const &v) const
        {
            if (v.type == EventTypes::EntityDestroyed)
            {
                const auto &event = static_cast<const EntityDestroyed &>(v);
                o.pack_array(4);
                o.pack(event.type);
                o.pack(event.get_id());
                o.pack(event.get_time());
                o.pack(event.get_transform());
            }
            else if (v.type == EventTypes::EffectTriggered)
            {
                const auto &event = static_cast<const EffectTriggered &>(v);
                o.pack_array(4);
                o.pack(event.type);
                o.pack(event.get_effect_type());
                o.pack(event.get_time());
                o.pack(event.get_transform());
            }
            else
            {
                throw std::runtime_error("Tried to serialize unknown event type");
            }
            return o;
        }
    };

    template <>
    struct pack<StateMessage>
    {
        template <typename Stream>
        packer<Stream> &operator()(msgpack::packer<Stream> &o, StateMessage const &v) const
        {
            o.pack_array(8);
            o.pack_array(1);
            o.pack(v.type);
            o.pack(v.agent_transforms);
            o.pack(v.entity_transforms);
            o.pack(v.events);
            o.pack(v.hps);
            o.pack(v.scores);
            o.pack(v.tick);
            o.pack(v.done);
            return o;
        }
    };

    template <>
    struct as<StateMessage>
    {
        StateMessage operator()(msgpack::object const &o) const
        {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 8)
                throw msgpack::type_error();
            std::vector<std::unique_ptr<IEvent>> events;
            const auto &events_array = o.via.array.ptr[3].via.array;
            for (unsigned int i = 0; i < events_array.size; ++i)
            {
                const auto &event = events_array.ptr[i];
                if (event.via.array.ptr[0].as<EventTypes>() == EventTypes::EntityDestroyed)
                {
                    events.push_back(std::make_unique<EntityDestroyed>(event.via.array.ptr[1].as<int>(),
                                                                       event.via.array.ptr[2].as<double>(),
                                                                       event.via.array.ptr[3].as<Transform>()));
                }
                else if (event.via.array.ptr[0].as<EventTypes>() == EventTypes::EffectTriggered)
                {
                    events.push_back(std::make_unique<EffectTriggered>(event.via.array.ptr[1].as<EffectTypes>(),
                                                                       event.via.array.ptr[2].as<double>(),
                                                                       event.via.array.ptr[3].as<Transform>()));
                }
            }
            return StateMessage(o.via.array.ptr[1].as<std::vector<Transform>>(),
                                o.via.array.ptr[2].as<std::unordered_map<unsigned int, Transform>>(),
                                std::move(events),
                                o.via.array.ptr[4].as<std::vector<float>>(),
                                o.via.array.ptr[5].as<std::vector<float>>(),
                                o.via.array.ptr[7].as<bool>(),
                                o.via.array.ptr[6].as<int>());
        }
    };
    }
}
}
