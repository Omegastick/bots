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

enum HpOrHit
{
    Hp = 0,
    Hit = 1,
};

struct RewardConfig
{
    RewardConfig();
    RewardConfig(nlohmann::json &json);

    float victory_reward = 100;
    float loss_punishment = -100;

    float hit_enemy_reward = 1;
    HpOrHit hit_enemy_type = HpOrHit::Hp;
    float hit_self_punishment = -1;
    HpOrHit hit_self_type = HpOrHit::Hp;

    float hill_tick_reward = 0.1;
    float enemy_hill_tick_punishment = -0.1;

    nlohmann::json to_json() const;
};

struct HyperParameters
{
    HyperParameters();
    HyperParameters(nlohmann::json &json);

    Algorithm algorithm = A2C;
    int batch_size = 2048;
    float discount_factor = 0.99;
    float entropy_coef = 0.001;
    int num_env = 8;
    float learning_rate = 0.0001;
    float actor_loss_coef = 0.5;
    float value_loss_coef = 0.5;

    // PPO
    float clip_param = 0.2;
    int num_epoch = 3;
    int num_minibatch = 32;

    nlohmann::json to_json() const;
};

struct TrainingProgram
{
    TrainingProgram();
    TrainingProgram(nlohmann::json &json);

    nlohmann::json agent;
    std::string checkpoint;
    HyperParameters hyper_parameters;
    int minutes_per_checkpoint = 10;
    RewardConfig reward_config;

    nlohmann::json to_json() const;
};
}