#pragma once

#include <msgpack.hpp>
#include <string>

namespace SingularityTrainer
{
const std::string API_VERSION = "v1alpha1";

template <class T>
struct Request
{
    Request(const std::string &method, const std::shared_ptr<T> param, const int id) : api(API_VERSION), method(method), param(param), id(id) {}

    std::string api;
    std::string method;
    std::shared_ptr<T> param;
    int id;
    MSGPACK_DEFINE_MAP(api, method, param, id)
};

struct GetActionsParam
{
    std::vector<std::vector<float>> inputs;
    int session_id;
    MSGPACK_DEFINE_MAP(inputs, session_id)
};

struct Model
{
    int inputs;
    int outputs;
    bool recurrent;
    bool normalize_observations;
    MSGPACK_DEFINE_MAP(inputs, outputs, recurrent, normalize_observations)
};

struct HyperParams
{
    float learning_rate;
    int batch_size;
    int num_minibatch;
    int epochs;
    float discount_factor;
    bool use_gae;
    float gae;
    float critic_coef;
    float entropy_coef;
    float max_grad_norm;
    float clip_factor;
    bool use_gpu;
    bool normalize_rewards;
    MSGPACK_DEFINE_MAP(
        learning_rate,
        batch_size,
        num_minibatch,
        epochs,
        discount_factor,
        use_gae,
        gae,
        critic_coef,
        entropy_coef,
        max_grad_norm,
        clip_factor,
        use_gpu,
        normalize_rewards)
};

struct BeginSessionParam
{
    Model model;
    HyperParams hyperparams;
    int session_id;
    bool training;
    int contexts;
    bool auto_train;
    MSGPACK_DEFINE_MAP(model, hyperparams, training, contexts, auto_train, session_id)
};

struct GiveRewardsParam
{
    std::vector<float> rewards;
    std::vector<bool> dones;
    int session_id;
    MSGPACK_DEFINE_MAP(rewards, dones, session_id)
};

struct EndSessionParam
{
    int session_id;
    MSGPACK_DEFINE_MAP(session_id)
};

template <class T>
struct Response
{
    std::string api;
    T result;
    int id;
    MSGPACK_DEFINE_MAP(api, result, id)
};

typedef std::string BeginSessionResult;
typedef std::string EndSessionResult;
typedef std::string GiveRewardsResult;

struct GetActionsResult
{
    std::vector<std::vector<int>> actions;
    std::vector<float> values;
    MSGPACK_DEFINE_MAP(actions, values)
};
}
