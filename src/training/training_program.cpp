#include <stdexcept>

#include <doctest.h>
#include <nlohmann/json.hpp>

#include "training/training_program.h"

namespace SingularityTrainer
{
static const std::string schema_version = "v1alpha3";

TrainingProgram::TrainingProgram() {}
HyperParameters::HyperParameters() {}
RewardConfig::RewardConfig() {}

TrainingProgram::TrainingProgram(nlohmann::json &json)
{
    if (json["schema"] != schema_version)
    {
        throw std::runtime_error("Invalid schema version");
    }
    body = json["body"];
    checkpoint = json["checkpoint"];
    hyper_parameters = json["hyper_parameters"];
    minutes_per_checkpoint = json["minutes_per_checkpoint"];
    opponent_pool = json["opponent_pool"].get<std::vector<std::string>>();
    reward_config = json["reward_config"];
}

HyperParameters::HyperParameters(nlohmann::json &json)
{
    algorithm = json["algorithm"];
    batch_size = json["batch_size"];
    discount_factor = json["discount_factor"];
    entropy_coef = json["entropy_coef"];
    num_env = json["num_env"];
    learning_rate = json["learning_rate"];
    actor_loss_coef = json["actor_loss_coef"];
    value_loss_coef = json["value_loss_coef"];
    clip_param = json["clip_param"];
    num_epoch = json["num_epoch"];
    num_minibatch = json["num_minibatch"];
}

RewardConfig::RewardConfig(nlohmann::json &json)
{
    victory_reward = json["victory_reward"];
    loss_punishment = json["loss_punishment"];
    hit_enemy_reward = json["hit_enemy_reward"];
    hit_enemy_type = json["hit_enemy_type"];
    hit_self_punishment = json["hit_self_punishment"];
    hit_self_type = json["hit_self_type"];
    hill_tick_reward = json["hill_tick_reward"];
    enemy_hill_tick_punishment = json["enemy_hill_tick_punishment"];
}

nlohmann::json TrainingProgram::to_json() const
{
    nlohmann::json json;

    json["schema"] = schema_version;
    json["body"] = body;
    json["checkpoint"] = checkpoint;
    json["hyper_parameters"] = hyper_parameters.to_json();
    json["minutes_per_checkpoint"] = minutes_per_checkpoint;
    json["opponent_pool"] = opponent_pool;
    json["reward_config"] = reward_config.to_json();

    return json;
}

nlohmann::json HyperParameters::to_json() const
{
    nlohmann::json json;

    json["algorithm"] = algorithm;
    json["batch_size"] = batch_size;
    json["discount_factor"] = discount_factor;
    json["entropy_coef"] = entropy_coef;
    json["num_env"] = num_env;
    json["learning_rate"] = learning_rate;
    json["actor_loss_coef"] = actor_loss_coef;
    json["value_loss_coef"] = value_loss_coef;
    json["clip_param"] = clip_param;
    json["num_epoch"] = num_epoch;
    json["num_minibatch"] = num_minibatch;

    return json;
}

nlohmann::json RewardConfig::to_json() const
{
    nlohmann::json json;

    json["victory_reward"] = victory_reward;
    json["loss_punishment"] = loss_punishment;
    json["hit_enemy_reward"] = hit_enemy_reward;
    json["hit_enemy_type"] = hit_enemy_type;
    json["hit_self_punishment"] = hit_self_punishment;
    json["hit_self_type"] = hit_self_type;
    json["hill_tick_reward"] = hill_tick_reward;
    json["enemy_hill_tick_punishment"] = enemy_hill_tick_punishment;

    return json;
}

TEST_CASE("TrainingProgram")
{
    SUBCASE("Loaded Json has correct values")
    {
        nlohmann::json json;
        json["schema"] = schema_version;
        json["body"] = nlohmann::json("Body");
        json["checkpoint"] = "12345";
        json["minutes_per_checkpoint"] = 54321;
        json["opponent_pool"] = {"asd", "sdf"};

        nlohmann::json hyper_parameters;
        hyper_parameters["algorithm"] = 0;
        hyper_parameters["batch_size"] = 4;
        hyper_parameters["discount_factor"] = 0.2;
        hyper_parameters["entropy_coef"] = 0.3;
        hyper_parameters["num_env"] = 2;
        hyper_parameters["learning_rate"] = 2.2;
        hyper_parameters["actor_loss_coef"] = 100.;
        hyper_parameters["value_loss_coef"] = 22.3;
        hyper_parameters["clip_param"] = 0.3;
        hyper_parameters["num_epoch"] = 34;
        hyper_parameters["num_minibatch"] = 2;
        json["hyper_parameters"] = hyper_parameters;

        nlohmann::json reward_config;
        reward_config["victory_reward"] = 23;
        reward_config["loss_punishment"] = -23;
        reward_config["hit_enemy_reward"] = 3;
        reward_config["hit_enemy_type"] = 0;
        reward_config["hit_self_punishment"] = -40;
        reward_config["hit_self_type"] = 1;
        reward_config["hill_tick_reward"] = 2.3;
        reward_config["enemy_hill_tick_punishment"] = 2;
        json["reward_config"] = reward_config;

        TrainingProgram program(json);
        CHECK(program.body == nlohmann::json("Body"));
        CHECK(program.checkpoint == "12345");
        CHECK(program.minutes_per_checkpoint == 54321);
        CHECK(program.opponent_pool == std::vector<std::string>{"asd", "sdf"});

        CHECK(program.hyper_parameters.algorithm == Algorithm::A2C);
        CHECK(program.hyper_parameters.batch_size == doctest::Approx(4));
        CHECK(program.hyper_parameters.discount_factor == doctest::Approx(0.2));
        CHECK(program.hyper_parameters.entropy_coef == doctest::Approx(0.3));
        CHECK(program.hyper_parameters.num_env == doctest::Approx(2));
        CHECK(program.hyper_parameters.learning_rate == doctest::Approx(2.2));
        CHECK(program.hyper_parameters.actor_loss_coef == doctest::Approx(100.));
        CHECK(program.hyper_parameters.value_loss_coef == doctest::Approx(22.3));
        CHECK(program.hyper_parameters.clip_param == doctest::Approx(0.3));
        CHECK(program.hyper_parameters.num_epoch == doctest::Approx(34));
        CHECK(program.hyper_parameters.num_minibatch == doctest::Approx(2));

        CHECK(program.reward_config.victory_reward == doctest::Approx(23));
        CHECK(program.reward_config.loss_punishment == doctest::Approx(-23));
        CHECK(program.reward_config.hit_enemy_reward == doctest::Approx(3));
        CHECK(program.reward_config.hit_enemy_type == HpOrHit::Hp);
        CHECK(program.reward_config.hit_self_punishment == doctest::Approx(-40));
        CHECK(program.reward_config.hit_self_type == HpOrHit::Hit);
        CHECK(program.reward_config.hill_tick_reward == doctest::Approx(2.3));
        CHECK(program.reward_config.enemy_hill_tick_punishment == doctest::Approx(2));
    }

    SUBCASE("Can be converted to Json and back")
    {
        TrainingProgram program;
        program.body = nlohmann::json("Body");
        program.checkpoint = "12345";
        program.minutes_per_checkpoint = 54321;
        program.opponent_pool = {"asd", "sdf"};

        program.hyper_parameters.algorithm = Algorithm::A2C;
        program.hyper_parameters.batch_size = 4;
        program.hyper_parameters.discount_factor = 0.2;
        program.hyper_parameters.entropy_coef = 0.3;
        program.hyper_parameters.num_env = 2;
        program.hyper_parameters.learning_rate = 2.2;
        program.hyper_parameters.actor_loss_coef = 100.;
        program.hyper_parameters.value_loss_coef = 22.3;
        program.hyper_parameters.clip_param = 0.3;
        program.hyper_parameters.num_epoch = 34;
        program.hyper_parameters.num_minibatch = 2;

        program.reward_config.victory_reward = 23;
        program.reward_config.loss_punishment = -23;
        program.reward_config.hit_enemy_reward = 3;
        program.reward_config.hit_enemy_type = HpOrHit::Hp;
        program.reward_config.hit_self_punishment = -40;
        program.reward_config.hit_self_type = HpOrHit::Hit;
        program.reward_config.hill_tick_reward = 2.3;
        program.reward_config.enemy_hill_tick_punishment = 2;

        auto json = program.to_json();
        TrainingProgram recreated_program(json);

        CHECK(recreated_program.body == nlohmann::json("Body"));
        CHECK(recreated_program.checkpoint == "12345");
        CHECK(recreated_program.minutes_per_checkpoint == 54321);
        CHECK(recreated_program.opponent_pool == std::vector<std::string>{"asd", "sdf"});

        CHECK(recreated_program.hyper_parameters.algorithm == Algorithm::A2C);
        CHECK(recreated_program.hyper_parameters.batch_size == doctest::Approx(4));
        CHECK(recreated_program.hyper_parameters.discount_factor == doctest::Approx(0.2));
        CHECK(recreated_program.hyper_parameters.entropy_coef == doctest::Approx(0.3));
        CHECK(recreated_program.hyper_parameters.num_env == doctest::Approx(2));
        CHECK(recreated_program.hyper_parameters.learning_rate == doctest::Approx(2.2));
        CHECK(recreated_program.hyper_parameters.actor_loss_coef == doctest::Approx(100.));
        CHECK(recreated_program.hyper_parameters.value_loss_coef == doctest::Approx(22.3));
        CHECK(recreated_program.hyper_parameters.clip_param == doctest::Approx(0.3));
        CHECK(recreated_program.hyper_parameters.num_epoch == doctest::Approx(34));
        CHECK(recreated_program.hyper_parameters.num_minibatch == doctest::Approx(2));

        CHECK(recreated_program.reward_config.victory_reward == doctest::Approx(23));
        CHECK(recreated_program.reward_config.loss_punishment == doctest::Approx(-23));
        CHECK(recreated_program.reward_config.hit_enemy_reward == doctest::Approx(3));
        CHECK(recreated_program.reward_config.hit_enemy_type == HpOrHit::Hp);
        CHECK(recreated_program.reward_config.hit_self_punishment == doctest::Approx(-40));
        CHECK(recreated_program.reward_config.hit_self_type == HpOrHit::Hit);
        CHECK(recreated_program.reward_config.hill_tick_reward == doctest::Approx(2.3));
        CHECK(recreated_program.reward_config.enemy_hill_tick_punishment == doctest::Approx(2));
    }
}
}