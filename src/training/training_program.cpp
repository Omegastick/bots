#include <doctest.h>
#include <nlohmann/json.hpp>

#include "training/training_program.h"

namespace SingularityTrainer
{
TrainingProgram::TrainingProgram() {}
HyperParameters::HyperParameters() {}
RewardConfig::RewardConfig() {}

TrainingProgram::TrainingProgram(nlohmann::json &json)
    : agent(json["agent"]),
      checkpoint(json["checkpoint"]),
      hyper_parameters(json["hyper_parameters"]),
      reward_config(json["reward_config"]) {}

HyperParameters::HyperParameters(nlohmann::json &json) {}
RewardConfig::RewardConfig(nlohmann::json &json) {}

bool TrainingProgram::operator==(const TrainingProgram &other) const { return false; }
bool HyperParameters::operator==(const HyperParameters &other) const { return false; }
bool RewardConfig::operator==(const RewardConfig &other) const { return false; }

nlohmann::json TrainingProgram::to_json() const { return {}; }
nlohmann::json HyperParameters::to_json() const { return {}; }
nlohmann::json RewardConfig::to_json() const { return {}; }

TEST_CASE("TrainingProgram")
{
    SUBCASE("Loaded Json has correct values")
    {
        nlohmann::json json;
        json["agent"] = nlohmann::json("Agent");
        json["checkpoint"] = "12345";

        nlohmann::json hyper_parameters;
        hyper_parameters["algorithm"] = 0;
        hyper_parameters["batch_size"] = 4;
        hyper_parameters["entropy_coef"] = 0.3;
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
        CHECK(program.agent == nlohmann::json("Agent"));
        CHECK(program.checkpoint == "12345");

        CHECK(program.hyper_parameters.algorithm == 0);
        CHECK(program.hyper_parameters.batch_size == 4);
        CHECK(program.hyper_parameters.entropy_coef == 0.3);
        CHECK(program.hyper_parameters.learning_rate == 2.2);
        CHECK(program.hyper_parameters.actor_loss_coef == 100.);
        CHECK(program.hyper_parameters.value_loss_coef == 22.3);
        CHECK(program.hyper_parameters.clip_param == 0.3);
        CHECK(program.hyper_parameters.num_epoch == 34);
        CHECK(program.hyper_parameters.num_minibatch == 2);

        CHECK(program.reward_config.victory_reward == 23);
        CHECK(program.reward_config.loss_punishment == -23);
        CHECK(program.reward_config.hit_enemy_reward == 3);
        CHECK(program.reward_config.hit_enemy_type == HpOrHit::Hp);
        CHECK(program.reward_config.hit_self_punishment == -40);
        CHECK(program.reward_config.hit_self_type == HpOrHit::Hit);
        CHECK(program.reward_config.hill_tick_reward == 2.3);
        CHECK(program.reward_config.enemy_hill_tick_punishment == 2);
    }

    SUBCASE("Can be converted to Json and back")
    {
        TrainingProgram program;
        program.agent = nlohmann::json("Agent");
        program.checkpoint = "12345";

        program.hyper_parameters.algorithm = Algorithm::A2C;
        program.hyper_parameters.batch_size = 4;
        program.hyper_parameters.entropy_coef = 0.3;
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

        CHECK(recreated_program == program);
    }
}
}