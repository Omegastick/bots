#pragma once

#include <memory>

#include <nlohmann/json_fwd.hpp>

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
    TrainingProgram(nlohmann::json &json);

    std::unique_ptr<Agent> agent;
    std::string checkpoint;
    Algorithm algorithm;
    std::unique_ptr<HyperParameters> hyper_parameters;

    nlohmann::json to_json() const;
};
}