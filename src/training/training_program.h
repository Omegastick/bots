#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace ai
{
enum class Algorithm
{
    A2C = 0,
    PPO = 1
};

enum class HpOrHit
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

    float hill_tick_reward = 0.1f;
    float enemy_hill_tick_punishment = -0.1f;

    nlohmann::json to_json() const;
};

struct HyperParameters
{
    HyperParameters();
    HyperParameters(nlohmann::json &json);

    Algorithm algorithm = Algorithm::A2C;
    int batch_size = 128;
    float discount_factor = 0.99f;
    float entropy_coef = 0.001f;
    int num_env = 8;
    float learning_rate = 0.0007f;
    float actor_loss_coef = 0.666f;
    float value_loss_coef = 0.333f;

    // PPO
    float clip_param = 0.1f;
    int num_epoch = 3;
    int num_minibatch = 8;

    nlohmann::json to_json() const;
};

struct TrainingProgram
{
    TrainingProgram();
    TrainingProgram(nlohmann::json &json);

    nlohmann::json body;
    std::string checkpoint;
    HyperParameters hyper_parameters;
    int minutes_per_checkpoint = 1;
    std::vector<std::string> opponent_pool;
    RewardConfig reward_config;

    nlohmann::json to_json() const;
};
}