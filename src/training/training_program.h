#pragma once

#include <memory>

#include <nlohmann/json.hpp>

namespace SingularityTrainer
{
class Agent;

enum Algorithm
{
    A2C,
    PPO
};

struct HyperParameters
{
};

struct TrainingProgram
{
    TrainingProgram();
    TrainingProgram(nlohmann::json &json);

    nlohmann::json agent;
    std::string checkpoint;
    Algorithm algorithm;
    std::unique_ptr<HyperParameters> hyper_parameters;

    nlohmann::json to_json() const;
};
}