#pragma once

#include <memory>

#include <nlohmann/json.hpp>

namespace SingularityTrainer
{
class Agent;

enum Algorithm
{
    A2C = 0,
    PPO = 1
};

struct HyperParameters
{
    Algorithm algorithm;
    int batch_size = 2048;
    float discount_factor = 0.99;
    float entropy_coef = 0.001;
    float learning_rate = 0.0001;
    float actor_loss_coef = 0.5;
    float value_loss_coef = 0.5;

    // PPO
    float clip_param = 0.2;
    int num_epoch = 3;
    int num_minibatch = 32;
};

struct TrainingProgram
{
    TrainingProgram();
    TrainingProgram(nlohmann::json &json);

    nlohmann::json agent;
    std::string checkpoint;
    HyperParameters hyper_parameters;

    nlohmann::json to_json() const;
};
}