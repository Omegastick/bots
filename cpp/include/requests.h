#pragma once

#include <msgpack.hpp>
#include <string>

namespace SingularityTrainer
{
template <class T>
struct Request
{
    Request(const std::string &method, const std::shared_ptr<T> param, const int id)
    {
        this->api = "v1alpha1";
        this->method = method;
        this->param = param;
        this->id = id;
    };

    std::string api;
    std::string method;
    std::shared_ptr<T> param;
    int id;
    MSGPACK_DEFINE_MAP(api, method, param, id);
};

struct GetActionsParam
{
    std::vector<std::vector<float>> inputs;
    int session_id;
    MSGPACK_DEFINE_MAP(inputs, session_id);
};

struct Model
{
    int inputs;
    int outputs;
    MSGPACK_DEFINE_MAP(inputs, outputs);
};

struct HyperParams
{
    float learning_rate;
    float gae;
    float batch_size;
    MSGPACK_DEFINE_MAP(learning_rate, gae, batch_size);
};

struct BeginSessionParam
{
    Model model;
    HyperParams hyperparams;
    int session_id;
    bool training;
    int contexts;
    bool auto_train;
    MSGPACK_DEFINE_MAP(model, hyperparams, training, contexts, auto_train, session_id);
};

struct GiveRewardsParams
{
    std::vector<float> rewards;
    std::vector<bool> dones;
    int session_id;
    MSGPACK_DEFINE_MAP(rewards, dones, session_id);
};

struct EndSessionParams
{
    int session_id;
    MSGPACK_DEFINE_MAP(session_id);
};
}
